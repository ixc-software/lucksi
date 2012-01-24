'''
Created on Feb 12, 2010

@author: root
'''
from PyQt4 import QtCore, QtGui
from CDUtilsPack.MetaUtils import Property

from DRIDomain.TelnetClientDri import DriError
from DRIDomain.TelnetClientDriAsync import TelnetClientDriAsync, CommandDriHandler
from DRIDomain.TelnetClientDriAsyncQueue import TelnetClientDriAsyncQueue

from QConfigMain.CoreDeviceBroadcastInfo import CoreDeviceBroadcastInfo
from PackUtils.CorePathes import TempFiles, IconFiles
from QConfigMain.GuiDeviceConfig import (GuiDeviceConfig, GuiDeviceConfigHandler,
                                  GuiDeviceConfigErr)
from QConfigMain.GuiDeviceCommon import GuiDeviceCommon                                  
from QConfigMain.GuiDeviceFactory import GuiDeviceFactory, GuiDeviceFactoryErr
from QConfigMain.GuiDeviceWidget import GuiDeviceWidget, WidgetMaker
from PackUtils.CoreTrace import CoreTrace
from QConfigMain.CoreFieldValidator import ValidatorErr
from QConfigMain.CoreFieldConverter import ConverterErr
from CDUtilsPack.Telnet.TelnetParams import TelnetParams
from PackUtils.CoreBaseException import CoreBaseException
import threading

# ------------------------------------------------------------

def _ShowErrBox(parent : QtGui.QWidget, text : str):
    QtGui.QMessageBox.warning(parent, "Error", text, QtGui.QMessageBox.Ok)

# ------------------------------------------------------------

class DeviceNameError(CoreBaseException):
    def __init__(self, text : str):
        Exception.__init__(self, text)

# ------------------------------------------------------------

class GuiDevice(GuiDeviceWidget):
    """Common interface of all devices used by GuiDeviceList."""
    def __init__(self,
                 nameCheckerFn,
                 info : CoreDeviceBroadcastInfo,
                 confDict : {},
                 devConfigDir : str,  
                 dri : TelnetClientDriAsync,
                 fact : GuiDeviceFactory,
                 statusFn, 
                 errorHandler, 
                 trace : CoreTrace):
        """
        'nameCheckerFn' - fn(name: str, staticNAme : str) -> bool, using to 
                          check that device name is unique within device list,
        'info'          - broadcast info,
        'confDict'      - loaded from file config, can be None,
        'devConfigDir'  - directory using to save device config file,  
        'statusFn'      - fn(name : str, text : str) -> None,
        'errorHandler'  - fn(e : Exception) -> None  
        
        If device created from config file: info == None and 
        info part of widget will be updated on next broadcast.
        If device created from broadcast info: confDict == None, widget 
        will be updated from default config.
        """
        uiPyModule = TempFiles.UiPyModule('QConfigMain', 'DeviceWidget')
        GuiDeviceWidget.__init__(self, None, uiPyModule, 'Ui_DeviceWidget')
        
        self.errorHandler = errorHandler
        
        self.vertScroll = None # to adjust on resize
        self.runButton = None  # QAction, using to change: 'Run' <-> 'Stop'
        self.monitButton = None
        
        self.dri = TelnetClientDriAsyncQueue(dri, self.errorHandler)
        self.nameCheckerFn = nameCheckerFn
        self.processedMonitCmdCount = 0
        self.isRunnning = False
        
        self.info = info
        self.conf = None
        self.name = None
        self.staticName = None
        self.hwNumber = None
        self.hwID = None
        
        self.common = None
        self.custom = None
        
        self.inClosingState = False
                
        self.threadId = threading.current_thread()
        
        self.__InitProperties(info, confDict)
        self.__InitCommonPart(statusFn, trace)
        self.__InitCustomPart(info, confDict, devConfigDir, fact)
        self.__SetupUi(fact)
        self.__RegisterInHwFinder()
        
        if self.info: self.UpdateBroadcastInfo(self.info)
    
    
    def __del__(self):
        assert self.inClosingState                 
       
    
    def __InitProperties(self, info : CoreDeviceBroadcastInfo, confDict : {}):
        if info:
            self.name = info.Name
            self.staticName = info.StaticName
            self.hwNumber = info['HwNumber']
            self.hwID = info['HwID']
            if not self.name: self.name = info.StaticName                
        else:
            assert confDict
            self.name = confDict['devName']
            self.staticName = confDict['devStaticName']
            self.hwNumber = confDict['hwNumber']
            self.hwID = confDict['hwID']
            if not self.name: self.name = confDict['devStaticName']      
    
    
    def __InitCommonPart(self, statusFn, trace : CoreTrace):
        self.common = GuiDeviceCommon(self.StaticName, statusFn, trace)
    
    
    def __InitCustomPart(self, info : CoreDeviceBroadcastInfo, confDict : {}, 
                         devConfigDir : str, fact : GuiDeviceFactory):
        assert self.common
        
        try:
            # widget
            self.custom = fact.CreateCustomWidget(self.hwID, self.common, self)
            self.Ui.mainLayout.addWidget(self.custom, 3, 0)
            # config handler
            confFileName = GuiDeviceConfig.FileName(devConfigDir, self.StaticName)
            
            
            self.conf = \
                fact.CreateConfigHandler(self.hwID, confFileName, self.info, 
                                         confDict, self.custom, 
                                         self.__ExecDeviceCmd)
        except GuiDeviceFactoryErr:
            assert self.HasUnknownType
        
        # overwrite generated in info device alias (name)
        if info:
            self.name = "{0}_{1}".format(self.__Type, info["HwNumber"])          
    
    
    def __SetupUi(self, fact : GuiDeviceFactory):
        meta = fact.FieldWidgetTypes
        m = WidgetMaker(self.common, self, meta, False)
        
        # setup tool bar
        toolBarName = 'toolBar' 
        m.AddToolBar(toolBarName)
        
        AddAct = m.AddToolBarBtn
        useSep = True
        useOnRun = True
        useOnStop = True
        AddAct(toolBarName,   'Default', self.__OnDefaultPressed, 
                                         not useSep, not useOnRun, useOnStop,
                                         IconFiles.Default)
        AddAct(toolBarName,      'Undo', self.__OnUndoPressed,    
                                         not useSep, not useOnRun, useOnStop,
                                         IconFiles.Undo)
        AddAct(toolBarName,      'Save', self.__OnSavePressed,    
                                         not useSep, not useOnRun, useOnStop,
                                         IconFiles.Save)
        self.runButton = AddAct(toolBarName, 
                                  'Run', self.__OnRunStopPressed,
                                         useSep,  not useOnRun,  useOnStop,
                                         IconFiles.Run)
        
        self.monitButton = AddAct(toolBarName,    
                                  'Monitor', self.__OnMonitorPressed, 
                                          useSep, useOnRun,  not useOnStop,
                                          IconFiles.Monitor)
        # setup info         
        m.SetupShowWidgetBtn('btnInfo', 'infoAdv', 
                             ['groupCommon', 'groupInfo'], True)
        
        # setup name
        self.Ui.name.setText(self.StaticName)
        assert self.Name
        self.Ui.nameByUser.setText(self.Name)
        self.Ui.nameByUser.returnPressed.connect(self.__OnNameChanged)
        if not self.HasUnknownType: self.__OnNameChanged()
        #self.Ui.nameByUser.editingFinished.connect(self.__OnNameChanged)
        #m.EnableWidget('nameByUser', False, True)
    
    
    def __RegisterInHwFinder(self):
        cmd = 'HwFinder.Alias({0}, "{1}")'.\
                format(self.hwNumber, self.StaticName) 
        self.__ExecCmd(cmd)
    
    
    def closeEvent(self, event : QtGui.QCloseEvent): # override
        self.custom.close()                 
    
    
    @property
    def __Type(self) -> str:
        return "Unknown" if self.HasUnknownType else self.conf.DevType
    
    
    def __Trace(self, text : str) -> str:
        self.common.Trace(text)
    
    
    def __TraceDriRes(self, res : CommandDriHandler, cmd = ''):
        if not cmd: cmd = "<Unknown>"
        self.common.Trace("Executed: {0}\nResult:{1}".format(cmd, res))
    
    
    def __Status(self, text : str) -> str:
        self.common.Status(text)
    
    
    def __CheckThread(self):
        assert self.threadId == threading.current_thread()
        
    
    def __ExecCmd(self, cmd : str, callback = None):
        """
        Execute command, check it result AFTER calling 'callback' function.
        """
        if self.inClosingState: return
        
        def CallbackWithCheck(res : CommandDriHandler):
            self.__CheckThread()
            
            if (res.IsAborted): return
            
            if callback: callback(res)
            
            if not res.ResultsOK:
                self.errorHandler(DriError(res.LastResult, res.CommandsStr))
        
        self.dri.Process(cmd, CallbackWithCheck)
        
    
    def __ExecDeviceCmd(self, cmd : str, callback):
        # CS_Set Device
        self.__ExecCmd('CS_Set {0}'.format(self.StaticName))
        # .DeviceCommand
        self.__ExecCmd(cmd, callback)        
    
    
    def __OnRunPressed(self):
        runScript = self.conf.CurrDevName + " run script"
        
        def Completed(res : CommandDriHandler):
            self.__CheckThread()
            
            if (res.IsAborted): return
                            
            self.__TraceDriRes(res, "")
            if res.ResultsOK:
                self.__Status("run script completed")
                self.runButton.setText(self.runButton.tr('Stop'))
                self.Ui.toolBar.adjustSize()
                self.runButton.setIcon(QtGui.QIcon(IconFiles.Stop))
                self.conf.DeviceIsRunning(True)
                self.isRunnning = True
                self.runButton.setEnabled(True)
                self.IsRunningSig.emit(True)
                self.DescChangedSig.emit()                          
            else:
                self.__Status("run script completed with error")
                self.__OnStopPressed(True)
                self.errorHandler(DriError(res.LastResult, runScript))
        
        self.common.RunStarted()
        self.__TryRunScript("run", Completed)
    
    
    def __OnStopPressed(self, ignoreError = False):
        stopScript = self.conf.CurrDevName + " stop script"
        
        def Completed(res : CommandDriHandler):
            self.__CheckThread()
            if (res.IsAborted): return
            self.__TraceDriRes(res, stopScript)
            self.runButton.setEnabled(True)
            if res.ResultsOK or ignoreError:
                self.runButton.setText(self.runButton.tr('Run'))
                self.Ui.toolBar.adjustSize()
                self.runButton.setIcon(QtGui.QIcon(IconFiles.Run))
                self.__Status("stop script completed")
                self.conf.DeviceIsRunning(False)
                self.isRunnning = False
                self.IsRunningSig.emit(False)
                self.DescChangedSig.emit()
                self.common.StopCompleted()
            else:
                self.__Status("stop script completed with error")
                #self.common.StopCompleted()
                self.errorHandler(DriError(res.LastResult, stopScript))                
        
        self.__TryRunScript("stop", Completed)
    
    
    def __TryRunScript(self, scriptType : str, callback):
        scripts = {"run" : self.conf.RunScriptCommands, 
               "stop" : self.conf.StopScriptCommands}
        
        assert scriptType in scripts.keys()
        self.__Status("processing {} script ...".format(scriptType))
        try:
            self.dri.Process(scripts[scriptType](), callback)
        except (ValidatorErr, ConverterErr, GuiDeviceConfigErr) as e:
            self.common.StopCompleted() # enable run/stop button for using again
            self.__Status("{} script processing aborted".format(scriptType))
            self.errorHandler(e) # _ShowErrBox(self, str(e))
                
    
    def __OnRunStopPressed(self):
        """"Doesn't save current config to file, generate script 
        from current config."""
        self.runButton.setEnabled(False)
        
        if not self.isRunnning : self.__OnRunPressed()
        else: self.__OnStopPressed()
    
    
    def __OnUndoPressed(self):
        self.__Trace("Load .conf file")
                
        self.conf.WriteLastSavedToGui()
        self.name = self.conf.CurrDevName
        self.Ui.nameByUser.setText(self.Name)                
    
    
    def __SaveCurrConfig(self):
        self.__Trace("Save settings to .conf file")
        #self.name = self.Ui.nameByUser.text():
        #if self.info: self.info.Name = self.Ui.nameByUser.text()
        if self.name != self.Ui.nameByUser.text():
            self.__OnNameChanged()
        self.conf.WriteCurrToFile(self.name)
        
    
    def __OnSavePressed(self):
        try:
            self.__SaveCurrConfig()
        except (ValidatorErr, ConverterErr) as e:
            self.errorHandler(e) #_ShowErrBox(self, str(e))
    
    
    def __OnDefaultPressed(self):
        if not self.__IsOkToClose(): return      
        self.__Trace("Load default .conf file")
        self.conf.WriteDefaultToGui()        
    
    
    def __OnMonitorPressed(self):
        def MonitComleted():
            self.__CheckThread()
            self.processedMonitCmdCount += 1
            if self.processedMonitCmdCount < self.conf.MonitoringCommandsCount():
                return
            self.__Status("monitoring complete")
            self.processedMonitCmdCount = 0
        
        self.__Status("processing monitoring commands ...")
        self.conf.ProcessMonitoringCommands(MonitComleted)       
    
    
    def __OnNameChanged(self):
        def Error(text : str):
            text = text.format(newName)
            self.errorHandler(DeviceNameError(text)) #_ShowErrBox(self, text)
        
        newName = self.Ui.nameByUser.text()
        
        if self.nameCheckerFn(newName, self.StaticName):
            Error("Name '{}' already exists".format(newName))
            self.Ui.nameByUser.setText(self.Name)
            return
        
        if not newName: 
            Error("Name can't be empty")
            self.Ui.nameByUser.setText(self.Name)
            newName = self.Name
        
        self.name = newName
        if self.info: self.info.Name = newName
        self.conf.UpdateCurrDevName(newName) 
        self.DescChangedSig.emit()
        self.__Status("name changed")
    
    
    def __IsOkToClose(self, whyCloseInfo : str = None) -> bool:
        def HasUnsavedChanges() -> bool:
            if not self.isVisible(): return False
            sameName = (self.name == self.conf.CurrDevName)
            guiChanged = self.conf.IsGuiChanged()
            return (sameName and guiChanged)                   
        
        errInConfig = False
        hasChanges = False
        try:
            hasChanges = HasUnsavedChanges()
        except (ValidatorErr, ConverterErr) as e:
            errInConfig = True
        
        if not hasChanges and not errInConfig: return True
        
        Msg = QtGui.QMessageBox
        text = (whyCloseInfo + "\n\n") if whyCloseInfo else ''
        text += "Save configuration?"
        answer =  Msg.question(self, self.Name, text, Msg.Yes, Msg.No, Msg.Cancel)
        if answer == Msg.Cancel: 
            return False            
        
        if answer == Msg.Yes:
            try:
                self.__SaveCurrConfig()
            except (ValidatorErr, ConverterErr) as e:
                self.errorHandler(e) # _ShowErrBox(self, str(e))
                return False
        
        if answer == Msg.No:
            self.__OnUndoPressed()
            self.DescChangedSig.emit()
        
        return True        
   
    
    def ShortDescription(self) -> str:
        """Using to display in device list widget."""
        
        def Res(available : bool, ip : str = "") -> str:
            res = "{name} ({type} #{num}) [{is}available{ip}] [{state}]"
            params = {'name' : self.Name, 'type' : self.__Type, 
                      'num' : self.hwNumber, 'is' : "", 'ip' : "", 'state' : ""}
            params['state'] = "running" if self.IsRunning() else "stopped"
            if not available: params['is'] = "not "
            if ip: params['ip'] = ": " + ip
            return res.format(**params)
        
        if self.info: 
            return Res(True, self.info["IP"])
        
        return Res(False)
        
    
    def UpdateBroadcastInfo(self, newInfo : CoreDeviceBroadcastInfo):
        if not self.info: self.info = newInfo
        else: self.info.UpdateFrom(newInfo)
        
        self.Ui.info.setText(self.info.MainDesc(self.__Type))
        self.Ui.infoAdv.setText(self.info.AdvancedDesc('\n'))
    
    
    def Run(self):
        """Execute run script and remain in 'running' state."""
        if self.HasUnknownType: return
        
        assert not self.IsRunning()
        self.__OnRunStopPressed()
        
    
    def Stop(self):
        """Execute stop script and return to 'stopped' state."""
        if self.HasUnknownType: return
        
        if not self.IsRunning(): return
        self.__OnRunStopPressed()
    
    
    def IsRunning(self) -> bool:
        return self.isRunnning
    
    
    def Show(self, parent : QtGui.QScrollArea):
        """Show device widget on parent widget."""
        if self.HasUnknownType: return
        
        parent.setWidget(self)
        
        self.vertScroll = parent.verticalScrollBar()
        self.vertScroll.setMaximum(self.height())
        width = self.minimumWidth() + 4
        if self.vertScroll.isVisible: 
            width += self.vertScroll.width()
        #parent.setMinimumWidth(width)
        parent.setWidgetResizable(True)
        #parent.adjustSize()
        self.show()
        if self.IsRunning(): self.__Status("running")
        else: self.__Status("stopped")
    
    
    def TryHide(self, parent : QtGui.QScrollArea, 
                whyHideInfo : str = None) -> bool:
        """
        Trying to hide device widget(delete from parent widget).
        Asking to save setting to file if need.
        """
        assert parent
        if self.HasUnknownType: return True
        
        # save Name to config
        if self.Name != self.Ui.nameByUser.text():
            self.Ui.nameByUser.returnPressed.emit()
        
        if not self.__IsOkToClose(whyHideInfo): 
            return False
        
        self.hide()
        # remove self from parent
        parent.takeWidget()
        return True
    
    def TryClose(self, parent : QtGui.QScrollArea, 
                 whyCloseInfo :str = None) -> bool:
        """
        Doing the same action as TryHide(), but prevent user using 'Run' and
        'Monitor' buttons if 'Cancel' or 'No' pressed on pop-up dialog 
        (prevent using DRI). Display 'areYouSureMsg' in pop-up dialog.
        """
        self.inClosingState = True
        if not self.TryHide(parent, whyCloseInfo): 
            self.common.StopCompleted() # enable config buttons
            self.runButton.setEnabled(False)
            self.monitButton.setEnabled(False)
            return False
        
        if self.dri:
            self.dri.AbortAll()
            self.dri = None
        if self.custom: self.custom.close()
        
        return True

    
    def DeleteConfig(self):
        """
        Delete config file from disc. Abort all DRI commands. Remove device 
        alias from HwFinder"""
        #assert not self.IsRunning()
        if self.IsRunning(): self.Stop()
        
        cmd = 'HwFinder.Unalias("{0}")'.format(self.StaticName)
        
        def OnUnalias(res : CommandDriHandler):
            self.__CheckThread()
            if (res.IsAborted): return
            self.dri.AbortAll()
            if not res.ResultsOK: 
                self.errorHandler(DriError(res.LastResult, res.CommandsStr))
            
        self.__ExecCmd(cmd, OnUnalias)
        
        if not self.HasUnknownType: 
            self.conf.DeleteFile()
        
    
    def EnableTrace(self, state : bool):
        self.common.EnableTrace(state)
    
    
    @property
    def HasUnknownType(self) -> bool:
        return self.custom is None and self.conf is None
    
    
    Name = Property("name")
    StaticName = Property("staticName")
    
    DescChangedSig = QtCore.pyqtSignal()
    IsRunningSig = QtCore.pyqtSignal(bool)
        

# ----------------------------------------------
# Tests
# ----------------------------------------------


from TestDecor import UtTest
from AllTest.TestRunner import GTestRunner
from AllTest.TestParams import TestParams
from QConfigMain.CoreDeviceBroadcastInfo import CoreDeviceBroadcastInfoGen
from PackUtils.CorePathes import TempFiles 

    
@UtTest
def GuiDeviceTest():
    
    CTraceTest = False
    CShowWidget = CTraceTest 
    
    class Test:
            
        def __init__(self, p : TestParams, parentWidget : QtGui.QWidget):
            self.p = p
            
            fact = GuiDeviceFactory()
            Status = lambda name, text: self.p.Output(text)
            NameChecker = lambda name, staticName: None
            
            info = CoreDeviceBroadcastInfoGen(1001, 1)
            self.dev = GuiDevice(NameChecker, info, {}, TempFiles.DirTest, 
                                 self.p.DRI, fact, Status, lambda e: p.ErrHandler(e), 
                                 p.CreateCoreTrace(CTraceTest))
            
            def IsRunningSlot(state : bool):
#                if state:
#                    self.dev.DeleteConfig()
#                    self.dev.Stop()
#                else:
                if CShowWidget: return 
                
                self.dev.TryClose(parentWidget)
                self.p.Output("GuiDeviceTest: OK")
                self.p.Complete(closeApp = not CShowWidget)                     
            
            self.dev.IsRunningSig.connect(IsRunningSlot)
            
            self.__Run(parentWidget, self.p.Output)
        
        
        def __Run(self, parentWidget :  QtGui.QWidget, outputFn):
            assert self.dev.Name
            assert self.dev.StaticName
            
            self.dev.UpdateBroadcastInfo(CoreDeviceBroadcastInfoGen(1001, 1))
            self.dev.EnableTrace(True)
            
            outputFn(self.dev.ShortDescription())
            assert self.dev.ShortDescription()
            assert not self.dev.IsRunning()
            
            self.dev.Show(parentWidget)
            self.dev.TryHide(parentWidget)
            self.dev.Show(parentWidget)
            
            self.dev.Stop()
            self.dev.Run()
        
    
    def Impl(p : TestParams, parentWidget : QtGui.QWidget):
        t = Test(p, parentWidget)
    
    GTestRunner.RunGuiMainIpsiusDri(Impl, CShowWidget, QtGui.QScrollArea,
                                    traceTest = CTraceTest, enableDriTrace = CTraceTest,
                                    startEventLoop = True)

# ----------------------------------------------

if __name__ == "__main__":
    
    import unittest
    unittest.main()
    
    
        
    