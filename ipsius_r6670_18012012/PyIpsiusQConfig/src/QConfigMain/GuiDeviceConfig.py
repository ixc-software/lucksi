'''
Created on Mar 25, 2010

@author: root
'''

from PyQt4 import QtGui
from QConfigMain.CoreFieldValidator import Validator
from PackUtils.CoreBaseException import CoreBaseException
import os, pickle, glob
from CDUtilsPack.MetaUtils import Property

from QConfigMain.GuiDeviceConfigType import GuiDeviceMonitorItem
from QConfigMain.GuiDeviceWidget import GuiDeviceWidget 
from QConfigMain.GuiFieldWidgetFactory import GuiFieldWidgetFactory
from QConfigMain.GuiDeviceConfigField import GuiDeviceConfigField

# -----------------------------------------------   

class GuiDeviceConfigErr(CoreBaseException):
    def __init__(self, text : str):
        Exception.__init__(self, text) 


# -----------------------------------------------


class GuiDeviceConfig:
    """Customized device config."""
    def __init__(self, hwID : str, hwNumber : str, 
                 deviceName : str, deviceStaticName : str, 
                 customFields : [GuiDeviceConfigField], 
                 version : int, deviceType : str,
                 runScriptFileName : str, stopScriptFileName : str,
                 meta : GuiFieldWidgetFactory):
        """
        Created from info from broadcast or saved on disc config:
        'hwID' - device type ID, 
        'hwNumber' - device number, 
        'deviceName' - device name defined by user, used only in GUI, 
        'deviceStaticName' - unchangeable device name, used when generate 
                             device specific files, in DRI scripts, etc.
        Info from GuiDeviceConfigType:
        'customFields' - list of fields defined in Devices/<Type>/Config.py,
        'version' - config version, 
        'deviceType' - device type description,
        Other: 
        'runScriptFileName' - absolute path to current device type run script,
        'stopScriptFileName' - absolute path to current device type stop script.
        """
        self.version = version
        
        self.hwID = hwID
        self.hwNumber = hwNumber
        self.devName = deviceName
        self.devStaticName = deviceStaticName
        self.devType = deviceType   
        self.fields = customFields
        self.meta = meta
        self.runScriptFileName = runScriptFileName
        self.stopScriptFileName = stopScriptFileName  
    
    
    def __ExtractCmds(self, filePath : str, params : {}) -> [str]:
            
        def Format(dataList : [str]) -> [str]:
            
            def SimplifyLine(raw : str) -> str:
                raw = raw.strip()
                quoteCount = 0 
                for i, ch in enumerate(raw):
                    if ch == '"': quoteCount += 1
                    if (quoteCount % 2) == 0:  # outside string
                        if raw[i:].startswith("//"):
                            return raw[:i].strip()
                return raw
            
            res = []
            for line in dataList:
                line = SimplifyLine(line)
                if not line: continue
                try:
                    res.append(line.format(**params))
                except KeyError as e:
                    err = "Unknown parameter {0} in file '{1}'".format(str(e), filePath)
                    raise GuiDeviceConfigErr(err)                                  
            return res
            
        def GetData() -> str:
            assert os.path.exists(filePath)
            with open(filePath, 'r') as f:
                return f.read()
            
        return Format(GetData().splitlines())
    
    
    def __GetRunScriptCmds(self) -> str:
        params = {}
        for f in self.fields:
            if f.UseInRunScript:
                params[f.Name] = f.Value
                       
        return self.__ExtractCmds(self.runScriptFileName, params)
    
    
    def __GetStopScriptCmds(self) -> str:
        params = {}
        for f in self.fields:
            if f.UseInStopScript:
                params[f.Name] = f.Value       
        return self.__ExtractCmds(self.stopScriptFileName, params)
    
    
    Version = Property('version')
    
    HwID            = Property("hwID")
    HwNumber        = Property("hwNumber")
    DevStaticName   = Property("devStaticName")
    DevName         = Property("devName", True)
    DevType         = Property("devType")
    
        
    def WriteToGui(self):
        """Write fields values to widgets."""
        for f in self.fields:
            f.Write()
    
    
    def WriteDefaultToGui(self):
        for f in self.fields:
            f.WriteDefault()
    
    
    def ReadFromGui(self):  # can raise ValidatorErr, ConverterErr
        """Read fields widgets."""
        for f in self.fields:
            f.Read()
    
    
    def SaveToFile(self, fileName : str):
        """Save simple config dictionary to file in format:
        {'hwID' : hwID, ..., 'fields' : {'BoardName' : boardName, ...}}"""
        def IsSimpleType(val) -> bool:
            return (isinstance(val, str) or
                    isinstance(val, int) or
                    isinstance(val, bool) or
                    val == None)            
        
        toWrite = {}
        for key in self.__dict__.keys():
            val = self.__dict__[key]
            if IsSimpleType(val): toWrite[key] = val
    
        toWriteFields = {}
        for f in self.fields:
            assert IsSimpleType(f.Value)
            if f.NeedSaveToFile:
                toWriteFields[f.Name] = f.Value
            else: 
                toWriteFields[f.Name] = f.Default
        toWrite['fields'] = toWriteFields
    
        with open(fileName, 'w+b') as f:
            pickle.dump(toWrite, f)    
    
    
    @staticmethod
    def LoadFromFile(fileName : str) -> {}:
        """Load simple config dictionary from file."""
        assert os.path.exists(fileName)
        res = None
        with open(fileName, 'r+b') as f:
            res = pickle.load(f)
        assert res
        return res
    
    
    def CustomFieldsSetup(self, customWidget : GuiDeviceWidget, execDriCmdFn):
        """"""
        assert customWidget
        for f in self.fields:
            if f.IsCustomized(): continue
            f.CustomSetup(customWidget, self.meta, execDriCmdFn)
    
    
    def SetupFromSimpleDict(self, confDict : {}):
        """Overwrite self fields values by 'confDict' values. 
        Think that self version is the last one and ignore 
        unknown fields in 'confDict'. Customize fields."""
        for key in self.__dict__.keys():
            if key != 'fields':
                if key in confDict.keys():
                    self.__dict__[key] = confDict[key]
                continue
            loadedFields = confDict['fields']
            for f in self.fields:
                if f.Name in loadedFields.keys():
                    f.SetValue(loadedFields[f.Name])
        
    
    def SetupFromFile(self , fileName : str, 
                      customWidget : GuiDeviceWidget,
                      execDriCmdFn):
        assert os.path.exists(fileName)
        loaded = GuiDeviceConfig.LoadFromFile(fileName)
        assert loaded['version'] == self.Version
        self.SetupFromSimpleDict(loaded)
        self.CustomFieldsSetup(customWidget, execDriCmdFn)                
    
    
    def IsSameCustomFieldsValues(self, confDict : {}) -> bool:
        """Compare just custom fields values. Assert that common 
        fields and version the same."""
        assert self.Version == confDict['version']
        assert (self.HwID == confDict['hwID'] and
                self.HwNumber == confDict['hwNumber'] and 
                self.DevStaticName == confDict['devStaticName'])
        
        if self.DevName != confDict['devName']: return False
        otherFields = confDict['fields']
        for f in self.fields:
            if not f.Name in otherFields.keys(): 
                continue
            if f.Value != otherFields[f.Name]:
                return False
        return True        
    
    
    def IsDefault(self) -> bool:
        """Check if Value != Default."""
        for f in self.fields:
            if f.Value != f.Default: return False
        return True
        
        
    def RunScriptCommands(self) -> [str]:
        """
        Parse run script file to the list of DRI commands. Format commands 
        with values from config fields.
        Commands in file should be written using valid DRI command syntax.
        Parameter's values that need to be replaced from config should be 
        put in {}. Only comments from the start of new line is allowed.  
        Sample:
            Obj.Method({param1}, {param2})
            // comment
            Obj.Property = {param3}
            CS_Set "{param4}.{param5}"                
        """
        return self.__GetRunScriptCmds()
        
    
    def StopScriptCommands(self) -> [str]:
        """
        Parse stop script file to the list of DRI commands. File format is 
        the same as for run script.
        """
        return self.__GetStopScriptCmds()
    
    
    def DeviceIsRunning(self, state : bool):
        """
        Notify fields about device state. Using to enable/disable fields 
        widgets in runtime. 
        """
        for f in self.fields:
            f.DeviceIsRunning(state)
    
    
    @staticmethod
    def FileName(dir : str, devStaticName : str) -> str:
        return os.path.join(dir, devStaticName + '.conf') 

# -----------------------------------------------

class GuiDeviceConfigHandler:
    """
    Using to modify device config via GUI, save it to file and 
    load it from it.
    """
    
    def __init__(self, 
                 confFileName : str, 
                 conf : GuiDeviceConfig,
                 monitorList : [GuiDeviceMonitorItem], 
                 customWidget : GuiDeviceWidget,
                 execDriCmdFn):
        """
        'confFileName' - name of config file to work with, 
        'conf'         - config, loaded from file or generated from broadcast info 
                         and linked to custom widget,
        'customWidget' - custom part of GuiDevice, created via GuiDeviceFactory,
        'monitorList'  - list of monitoring commands linked to widgets,
        'execDriCmdFn' - fn(cmd : str, callback) -> None.
        """
        self.fileName = confFileName
        self.conf = conf
        self.widget = customWidget
        self.execDriCmdFn = execDriCmdFn
        self.monitorList = monitorList
        
        # setup GUI
        self.WriteLastSavedToGui()   
    
    
    def __del__(self):
        self.conf.DeviceIsRunning(False)   
    
    
    def WriteLastSavedToGui(self):
        """Read last saved settings from file and write them to GUI."""
        if os.path.exists(self.fileName):
            self.conf.SetupFromFile(self.fileName, self.widget, 
                                    self.execDriCmdFn)
            self.conf.WriteToGui()            
        else:
            self.WriteDefaultToGui()        
    
    
    def WriteDefaultToGui(self):
        """Write default config to GUI."""
        self.conf.WriteDefaultToGui()            
    
    
    def WriteCurrToFile(self, devName : str):  # can raise ValidatorErr, ConverterErr
        """Read current config from GUI. Save it to file."""
        self.conf.ReadFromGui()
        self.conf.SaveToFile(self.fileName)
    
    
    def DeleteFile(self):
        """Delete config file from disc."""
        if os.path.exists(self.fileName): 
            os.remove(self.fileName)
            
            
    def IsGuiChanged(self) -> bool:  # can raise ValidatorErr, ConverterErr
        self.conf.ReadFromGui()
        if not os.path.exists(self.fileName):
            return not self.conf.IsDefault()
        lastSaved = GuiDeviceConfig.LoadFromFile(self.fileName)
        return not self.conf.IsSameCustomFieldsValues(lastSaved)
        
        
    def RunScriptCommands(self) -> str:  # can raise ValidatorErr, ConverterErr
        self.conf.ReadFromGui()
        return self.conf.RunScriptCommands()
        
        
    def StopScriptCommands(self) -> str:  # can raise ValidatorErr, ConverterErr
        self.conf.ReadFromGui()
        return self.conf.StopScriptCommands()
        
        
    def UpdateCurrDevName(self, name : str):
        self.conf.DevName = name
    
    
    def DeviceIsRunning(self, state : bool):
        self.conf.DeviceIsRunning(state)
    
    
    def ProcessMonitoringCommands(self, onCompleteCmdNotifyFn):
        """
        'onCompleteCmdNotifyFn' - fn() -> None, called after each command 
        result was processed. Using to know if this is the last command.
        """
        for i in self.monitorList:
            i.Execute(onCompleteCmdNotifyFn)
    
    
    def MonitoringCommandsCount(self) -> int:
        return len(self.monitorList)
    
    
    @property
    def DevType(self) -> str:
        return self.conf.DevType


    @property
    def CurrDevName(self) -> str:
        return self.conf.DevName


    @staticmethod
    def ReadAllFiles(dir : str) -> [{}]:
        """
        Load all config files in 'dir' folder. 
        Return list of simple config dictionaries.
        """
        res = []
        files = glob.glob(GuiDeviceConfig.FileName(dir, '*'))
        for file in files:
            if file[-5:] == '.conf':
                res.append(GuiDeviceConfig.LoadFromFile(file))
        return res

# -----------------------------------------------
# Tests
# -----------------------------------------------

import sys
from TestDecor import UtTest
from CDUtilsPack.MiscUtils import GetScriptDir
from PackUtils.CorePathes import TempFiles, TempFiles
from QConfigMain.GuiDeviceWidget import GuiDeviceWidget
from AllTest.TestRunner import GTestRunner
from AllTest.TestParams import TestParams


@UtTest
def GuiDeviceConfigTest():
    
    fieldVal = "_default_" 
    ExecDriCmdFn = lambda cmd, callback: None
    
    def CheckScriptParsing(commands : [str], outputFn):
        outputFn("CheckScriptParsing: \n" + "\n".join(commands))
        assert len(commands) > 0
        hasVal = False
        for cmd in commands:
            assert cmd
            if not "// not a comment" in cmd: assert not "//" in cmd
            if fieldVal in cmd: hasVal = True
                
        if not hasVal: assert 0 and "Parsing script file into commands failed"
    
    def ConfigTest(c : GuiDeviceConfig, customWidget : GuiDeviceWidget, 
                   fileName : str, outputFn):    
        assert c.Version == 1
        assert c.HwID == 1
        assert c.HwNumber == 2
        assert c.DevStaticName == 'statName'
        assert c.DevName == 'name'
        assert c.DevType == 'Test'        
        CheckScriptParsing(c.RunScriptCommands(), outputFn)
        CheckScriptParsing(c.StopScriptCommands(), outputFn)         
        assert c.IsDefault()
        
        # save
        c.SaveToFile(fileName)    
        # load (I)
        c.SetupFromFile(fileName, customWidget, ExecDriCmdFn)
        # load (II)
        confDict = GuiDeviceConfig.LoadFromFile(fileName)
        assert c.IsSameCustomFieldsValues(confDict)
        confDict['newFieldX'] = ''
        assert not c.SetupFromSimpleDict(confDict)
        confDict = GuiDeviceConfig.LoadFromFile(fileName)
        assert not c.SetupFromSimpleDict(confDict)
        c.CustomFieldsSetup(customWidget, ExecDriCmdFn)
        # work with gui
        c.WriteToGui()
        c.WriteDefaultToGui()
        c.ReadFromGui()
        c.DeviceIsRunning(True)
        c.DeviceIsRunning(False)    
    
    def HandlerTest(c : GuiDeviceConfig, customWidget : GuiDeviceWidget, 
                    fileName : str, outputFn):
        h = GuiDeviceConfigHandler(fileName, c, [], customWidget, ExecDriCmdFn)
        assert h.MonitoringCommandsCount() == 0
        assert h.DevType == 'Test'
        CheckScriptParsing(h.RunScriptCommands(), outputFn)
        CheckScriptParsing(h.StopScriptCommands(), outputFn)
        assert not h.IsGuiChanged()

        def GetConfCount() -> int:
            return len(GuiDeviceConfigHandler.ReadAllFiles(TempFiles.Dir))
        
        h.WriteLastSavedToGui()
        h.WriteDefaultToGui()            
        
        h.WriteCurrToFile(c.DevName)
        confCount = GetConfCount()
        
        h.DeleteFile()
        assert not os.path.exists(fileName)
        #assert GetConfCount() == (confCount - 1)
        assert GetConfCount() == confCount
                
        h.UpdateCurrDevName('newName')
        assert c.DevName == 'newName'
        assert h.CurrDevName == 'newName'
        
        h.DeviceIsRunning(True)
        
        OnCompleteCmdNotifyFn = lambda: None 
        h.ProcessMonitoringCommands(OnCompleteCmdNotifyFn)
    
    
    def Impl(p : TestParams):
        script = os.path.join(GetScriptDir('../AllTest'), 'TestScript.isc')
        module = TempFiles.UiPyModule('Test')
        customWidget = GuiDeviceWidget(None, module)
        
        field = GuiDeviceConfigField("param", None, fieldVal, Validator(), 
                                     useInRunScript = True, useInStopScript = True, 
                                     needSaveToFile = True, runtimeDriCommand = None,
                                     toolTip = 'tip', enum = [], help = '')
        
        c = GuiDeviceConfig(hwID = 1, hwNumber  = 2,
                            deviceName  = 'name', deviceStaticName = 'statName', 
                            customFields = {field}, version  = 1, deviceType = 'Test',
                            runScriptFileName = script, stopScriptFileName = script,
                            meta = GuiFieldWidgetFactory())
        
        fileName = GuiDeviceConfig.FileName(TempFiles.DirTest, c.DevStaticName)
        if os.path.exists(fileName): os.remove(fileName)
        
        outputFn = lambda text: p.Output(text)
        ConfigTest(c, customWidget, fileName, outputFn)
        HandlerTest(c, customWidget, fileName, outputFn)
        p.Complete()
        # Note: customized config tested in GuiDeviceFactoryTest
    
    GTestRunner.RunApp(Impl, traceTest = False)

# -----------------------------------------------

if __name__ == "__main__":
    
    import unittest
    unittest.main()
