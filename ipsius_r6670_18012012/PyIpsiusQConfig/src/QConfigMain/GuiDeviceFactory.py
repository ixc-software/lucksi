'''
Created on Mar 23, 2010

@author: root
'''

from PyQt4 import QtGui
import os
from CDUtilsPack.MetaUtils import Property
from CDUtilsPack.MiscUtils import LoadModuleFromFile, GetScriptDir

from CDUtilsPack.Telnet.TelnetParams import TelnetParams
from QConfigMain.CoreFieldValidator import ValidatorErr
from QConfigMain.CoreFieldConverter import ConverterErr
from PackUtils.CorePathes import TempFiles
from QConfigMain.CoreDeviceBroadcastInfo import CoreDeviceBroadcastInfo
from PackUtils.CoreBaseException import CoreBaseException
from QConfigMain.GuiDeviceWidget import GuiDeviceWidget, WidgetMaker
from QConfigMain.GuiFieldWidgetFactory import GuiFieldWidgetFactory
from QConfigMain.GuiDeviceCommon import GuiDeviceCommon
from QConfigMain.GuiDeviceConfig import GuiDeviceConfig, GuiDeviceConfigHandler
from QConfigMain.GuiDeviceConfigType import (GuiDeviceConfigType, ConfigMaker, 
                                      MonitorMaker, GuiDeviceMonitorItem)
from PackUtils.Misc import StrToInt

# -------------------------------------------------------------

_CDevicesFolder = '../Devices'

# -------------------------------------------------------------

class GuiDeviceTypeDescErr(CoreBaseException):
    def __init__(self, err : Exception, file : str = None):
        
        text = "{0} in file '{1}'".format(err, file) if file else err
        Exception.__init__(self, text)
        

# -------------------------------------------------------------

class _CDeviceFiles:
    
    RunScript   = 'Run.isc'
    StopScript  = 'Stop.isc'
    Config      = 'Config.py'
    CustomImpl  = 'WidgetImpl.py'
    Widget      = 'Widget.ui'
    

# -------------------------------------------------------------


class _GuiDeviceType:
    """
    Device type template. Load device modules from folder. Using to 
    create custom device config and custom device widget implementation 
    class.
    """
    def __init__(self, deviceFolder : str, meta : GuiFieldWidgetFactory):
        """
        'deviceFolder' - absolute path to device folder.
        """
        self.boardNameFn = None
        self.hwNumberFn = None
        self.meta = meta
        self.folderName = os.path.basename(deviceFolder)
        
        def DevFileAbs(name : str, checkExists = True) -> str:
            res = os.path.join(deviceFolder, name)
            if not checkExists: return res
            if os.path.exists(res): return res
            raise GuiDeviceTypeDescErr("File '{0}' is not found".format(res))
            
        self.runScriptName = DevFileAbs(_CDeviceFiles.RunScript)
        self.stopScriptName = DevFileAbs(_CDeviceFiles.StopScript)
        self.configFileName = DevFileAbs(_CDeviceFiles.Config)
        self.customImplFileName = DevFileAbs(_CDeviceFiles.CustomImpl, False)
        if not os.path.exists(self.customImplFileName):  
            self.customImplFileName = None
        
        self.confType = self.__MakeConfigType()
        
    
    def __MakeConfigType(self):
        
        def GetBoardName() -> str:
            assert self.boardNameFn
            return self.boardNameFn()
        
        def GetHwNumber() -> int:
            assert self.hwNumberFn
            return self.hwNumberFn()
         
        res = GuiDeviceConfigType()
        config = LoadModuleFromFile(self.configFileName)
        confMaker = ConfigMaker(res, GetBoardName, GetHwNumber)
        monitMaker = MonitorMaker(res)
        config.MakeConfig(confMaker)
        config.MakeMonitor(monitMaker)
        return res
    
    def __MakeDefaultConfig(self, hwID : str, hwNumber : str, devName : str, 
                            devStaticName : str, version : int, 
                            customWidget : GuiDeviceWidget,
                            execDriCmdFn) -> GuiDeviceConfig:
        self.boardNameFn = lambda: devStaticName
        self.hwNumberFn = lambda: int(hwNumber)
        
        try:
            fields = self.confType.ConfigFields(customWidget, self.meta,
                                                execDriCmdFn)
            res = GuiDeviceConfig(\
                    hwID, hwNumber, devName, devStaticName, fields, 
                    version, self.confType.DeviceTypeName, 
                    self.runScriptName, self.stopScriptName, self.meta)
        except (ValidatorErr, ConverterErr) as e:
            raise GuiDeviceTypeDescErr(e, self.configFileName)
            
        self.boardNameFn = None
        self.hwNumberFn = None
        return res
    
    
    RunScriptName   = Property('runScriptName')
    StopScriptName  = Property('stopScriptName')


    @property
    def DeviceTypeID(self) -> int:
        return self.confType.DeviceTypeID
    
    
    @property
    def DeviceTypeName(self) -> str:
        return self.confType.DeviceTypeName
    
    
    def CreateMonitorList(self, customWidget : GuiDeviceWidget, 
                          execDriCmdFn) -> [GuiDeviceMonitorItem]:
        return self.confType.MonitorList(customWidget, self.meta, execDriCmdFn)
    
    
    def CreateCustomWidget(self, common : GuiDeviceCommon, parent : GuiDeviceWidget) -> GuiDeviceWidget: 
        """
        Create custom widget using py-module generated from .ui file and
        customize it with code from WidgetImpl.py.
        """
        uiPyModule = TempFiles.UiPyModule(self.folderName)
        res = GuiDeviceWidget(parent, uiPyModule)
        if not self.customImplFileName:
            return res
        
        customModule = LoadModuleFromFile(self.customImplFileName)
        m = WidgetMaker(common, res, self.meta, True)
        try:
            customModule.CustomizeWidget(m)
        except Exception as e:
            raise GuiDeviceTypeDescErr("{1} in file '{0}'".
                                       format(self.customImplFileName, str(e)))
        return res                
    
    
    def CreateConfig(self, confFileName : str, 
                     info : CoreDeviceBroadcastInfo, confDict : {}, 
                     customWidget : GuiDeviceWidget, execDriCmdFn) -> GuiDeviceConfig:
        """
        Return customized GuiDeviceConfig, linked to widget and 
        updated to the latest version.
        """        
        version = self.confType.Version
        
        res = None
        if info:
            assert not confDict
            res = self.__MakeDefaultConfig(\
                    info['HwID'], info['HwNumber'], info.Name, info.StaticName, 
                    version, customWidget, execDriCmdFn)
            res.CustomFieldsSetup(customWidget, execDriCmdFn)
        if confDict:
            assert not info
            res = self.__MakeDefaultConfig(\
                    confDict['hwID'], confDict['hwNumber'], confDict['devName'], 
                    confDict['devStaticName'], version, customWidget, execDriCmdFn)            
            overwriteFile = False
            if version != confDict['version']:
                overwriteFile = True
                confDict['version'] = version
            res.SetupFromSimpleDict(confDict)
            res.CustomFieldsSetup(customWidget, execDriCmdFn)
            if overwriteFile:
                res.SaveToFile(confFileName)
        assert res
        return res  

# -------------------------------------------------------------

class GuiDeviceFactoryErr(CoreBaseException):
    pass

# -------------------------------------------------------------

class GuiDeviceFactory:
    """
    Device type factory. Using to create custom part of GuiDevice 
    and customized GuiDeviceConfig. Generate device type classes 
    for all devices described in separate folders in folder 
    ./src/Devices.  
    """
    def __init__(self):
        self.types = [] # [_GuiDeviceType]
        self.meta = GuiFieldWidgetFactory()
        
        def IsDeviceDir(files : [str]) -> bool:
            return _CDeviceFiles.Widget in files
        
        for dir, subDirs, files in os.walk(GetScriptDir(_CDevicesFolder)):
            if not IsDeviceDir(files): continue
            t = _GuiDeviceType(dir, self.meta)
            self.__CheckDeviceTypeIdAndName(t.DeviceTypeID, t.DeviceTypeName)
            self.types.append(t)
    
    
    def __CheckDeviceTypeIdAndName(self, hwID : str, typeName : str):
        hwID = StrToInt(hwID)
        for t in self.types:
            # check ID
            if t.DeviceTypeID == hwID: 
                err = "Type with ID = {0} already exists.".\
                       format(t.DeviceTypeID)
                raise GuiDeviceFactoryErr(err)
        
            # check name
            if t.DeviceTypeName == typeName: 
                err = "Type with name = '{0}' already exists.".\
                       format(t.DeviceTypeName)
                raise GuiDeviceFactoryErr(err)
        
    
    def __FindAndCheck(self, hwID : str) -> _GuiDeviceType:
        hwID = StrToInt(hwID)
        for t in self.types:
            if t.DeviceTypeID == hwID: return t
        
        raise GuiDeviceFactoryErr("Unknown HwID: {0}".format(hwID))        
    
    
    FieldWidgetTypes = Property('meta')
    
    
    def TypeIdList(self) -> [str]:
        res = []
        for type in self.types:
            res.append(type.DeviceTypeID)
        return res
    
    
    def CreateCustomWidget(self, hwID : str, 
                           common : GuiDeviceCommon,
                           parentWidget : GuiDeviceWidget) -> GuiDeviceWidget:
        """Create custom part of GuiDevice class."""
        type = self.__FindAndCheck(hwID)
        return type.CreateCustomWidget(common, parentWidget)
    
    
    def CreateConfig(self, hwID : str, confFileName : str, 
                     info : CoreDeviceBroadcastInfo, confDict : {},
                     customWidget : GuiDeviceWidget,
                     execDriCmdFn) -> GuiDeviceConfig:
        """
        Create config from broadcast info 'info' or config dictionary 
        'confDict' that was loaded from file. Link it to widget 
        'customWidget' and update to the latest version. 
        'execDriCmdFn' - fn(str, callback) -> execute DRI command for
                         current device. 
        """   
        type = self.__FindAndCheck(hwID)
        return type.CreateConfig(confFileName, info, confDict, customWidget,
                                 execDriCmdFn)
    
    
    def CreateConfigHandler(self, hwID : str, confFileName : str, 
                            info : CoreDeviceBroadcastInfo, confDict : {},
                            customWidget : GuiDeviceWidget, 
                            execDriCmdFn) -> GuiDeviceConfigHandler:
        """
        Create config from broadcast info 'info' or config dictionary 
        'confDict' that was loaded from file. Link it to widget 
        'customWidget' and update to the latest version. Return handler 
        for this config.
        'execDriCmdFn' - fn(str, callback) -> execute DRI command for
                         current device.
        """   
        type = self.__FindAndCheck(hwID)
        conf = type.CreateConfig(confFileName, info, confDict, customWidget,
                                 execDriCmdFn)
        monit = type.CreateMonitorList(customWidget, execDriCmdFn)
        return GuiDeviceConfigHandler(confFileName, conf, monit, customWidget,
                                      execDriCmdFn)
         

# -------------------------------------------------
# Tests
# -------------------------------------------------

from TestDecor import UtTest
from AllTest.TestRunner import GTestRunner
from AllTest.TestParams import TestParams
from QConfigMain.CoreDeviceBroadcastInfo import CoreDeviceBroadcastInfoGen
from DRIDomain.TelnetClientDri import DriError
from DRIDomain.TelnetClientDriAsync import (TelnetClientDriAsync, CommandDriHandler)
from DRIDomain.TelnetClientDriAsyncQueue import TelnetClientDriAsyncQueue 
from PackUtils.CorePathes import TempFiles 

from PackUtils.CoreTrace import CoreTrace 

# -------------------------------------------------

class _TestDevice(GuiDeviceWidget):
    """
    Class - wrapped for custom device widget (analog of GuiDevice). 
    Using to check if device type has correct description: 
    - all required files exists,
    - fields names, widgets names and default value is correct,
    - device described using correct classes and functions.
    """
    def __init__(self, hwID : str, hwNumber : str, fact : GuiDeviceFactory,
                 dri : TelnetClientDriAsync, outputFn, trace : CoreTrace, 
                 errHandler, completeFn):
        uiPyModule = TempFiles.UiPyModule('QConfigMain', 'DeviceWidget')
        GuiDeviceWidget.__init__(self, None, uiPyModule, 'Ui_DeviceWidget')
        
        self.dri = TelnetClientDriAsyncQueue(dri, errHandler)
        self.info = CoreDeviceBroadcastInfoGen(hwID, hwNumber)
        self.name = self.info.StaticName
        self.completeFn = completeFn
        self.outputFn = outputFn
        self.processedMonitCmdCount = 0
        self.common = GuiDeviceCommon(self.name, 
                                      lambda name, text : outputFn(text), 
                                      trace)
        # check CreateCustomWidget()
        self.custom = fact.CreateCustomWidget(hwID, self.common, self)
        self.Ui.mainLayout.addWidget(self.custom, 3, 0)
        
        self.conf = None
        self.__CheckCreateConfigAndConfigHandler(hwID, fact)
        
        # start test
        self.__Step("Test started")
        self.__ExecRunScript()
    
    
    def __del__(self):
        self.dri = None
    
    
    def __CheckCreateConfigAndConfigHandler(self, hwID : str, 
                                            fact : GuiDeviceFactory):
        
        def ExecCmd(cmd : str, callback):
            setCmd = 'CS_Set {0}'.format(self.name)
        
            def OnSet(res : CommandDriHandler):
                if (res.IsAborted): return
                
                if not res.ResultsOK: 
                    self.completeFn()
                    self.__DriErr(res, setCmd)
            
            self.dri.Process(setCmd, OnSet)
            self.dri.Process(cmd, callback)
        
        confDir = TempFiles.DirTest
        confDict = {} 
        confFileName = GuiDeviceConfig.FileName(confDir, self.name)
        
        # check CreateConfigHandler()
        self.conf = \
            fact.CreateConfigHandler(hwID, confFileName, self.info, 
                                     confDict, self.custom, ExecCmd)  
        
        # check CreateConfig()
        confImpl = fact.CreateConfig(hwID, confFileName, self.info, 
                                     confDict, self.custom, ExecCmd)
        assert confImpl and isinstance(confImpl, GuiDeviceConfig)
        
    
    def __Step(self, name : str):
        self.outputFn("{0} for type '{1}'".format(name, self.conf.DevType))
    
    
    def __DriErr(self, res : CommandDriHandler, text : str):
        self.completeFn(1)
        print(res)
        raise DriError(res.LastResult, text)        
    
    
    def __ExecRunScript(self):
        self.__Step("Step 1")
        
        def Completed(res : CommandDriHandler):
            if (res.IsAborted): return
            
            if not res.ResultsOK:
                self.__DriErr(res, "Run script")
            self.__Step("Step 2")
            self.conf.DeviceIsRunning(True)
            self.__ExecMonitoring()
        
        self.common.RunStarted()
        self.dri.Process(self.conf.RunScriptCommands(), Completed)
    
    
    def __ExecStopScript(self):
        
        def Completed(res : CommandDriHandler):
            if (res.IsAborted): return
            
            if not res.ResultsOK:
                self.__DriErr(res, "Stop script")
            self.__Step("Step 4")
            self.conf.DeviceIsRunning(False)
            self.common.StopCompleted()
            
            Undo = lambda: self.conf.WriteLastSavedToGui()
            Default = lambda: self.conf.WriteDefaultToGui()
            Save = lambda: self.conf.WriteCurrToFile(self.name)
            Delete = lambda: self.conf.DeleteFile()  
            
            Undo()
            Default()
            Save()
            Delete()
            
            #self.dri.AbortAll() # close
            self.__Step("Test completed")
            self.completeFn(0)
        
        self.dri.Process(self.conf.StopScriptCommands(), Completed)
        
    
    def __ExecMonitoring(self):
        
        def MonitComleted():
            self.processedMonitCmdCount += 1
            if self.processedMonitCmdCount < self.conf.MonitoringCommandsCount():
                return
            self.__Step("Step 3")
            self.processedMonitCmdCount = 0
            self.__ExecStopScript()            
        
        self.conf.ProcessMonitoringCommands(MonitComleted)

# -------------------------------------------------

def _GuiDeviceFactoryTest(traceTest : bool, 
                          ipsiusFilePath = GTestRunner.IpsiusPath):
    """Test GuiDeviceFactory and check all device type description."""
    
    CTraceTest = False
    
    devList = []
    
    def Impl(p : TestParams):
        hwNumber = 2
        fact = GuiDeviceFactory()
        assert fact.FieldWidgetTypes
        hwIdList = fact.TypeIdList()
        
        def CompleteIfError(code = 0):
            if code == 0: return
            p.Complete()
        
        for i in range(len(hwIdList)):
            testCompleteFn = CompleteIfError
            if i == len(hwIdList) - 1: 
                testCompleteFn = lambda code: p.Complete(code)
            dev = _TestDevice(fact.TypeIdList()[i], hwNumber, fact, p.dri, 
                              lambda text: p.Output(text), 
                              p.CreateCoreTrace(CTraceTest), 
                              lambda e: p.ErrHandler(), 
                              testCompleteFn)
            devList.append(dev)                        
    
    GTestRunner.RunIpsius(Impl, CTraceTest, startEventLoop = True, withDri = True, enableDriTrace = CTraceTest)
    
# -------------------------------------------------

def DevicesTypeCheck(exeFilePath : str):
    print("Running tests to check device types ...")
    print("Starting ProjIpsius for tests ... \n")
    
#    try:
#        _GuiDeviceFactoryTest(False, exeFilePath)
#    except Exception as e:
#        print("Failed!")
#        print(e)
#        return False
    
    _GuiDeviceFactoryTest(False, exeFilePath) # can raise Exception
    
    print("Complete.\n")
    

# -------------------------------------------------

@UtTest
def GuiDeviceFactoryTest():
    _GuiDeviceFactoryTest(traceTest = False)

# -------------------------------------------------

if __name__ == "__main__":
    
    import unittest
    unittest.main()    
    
    
    
    
    
    