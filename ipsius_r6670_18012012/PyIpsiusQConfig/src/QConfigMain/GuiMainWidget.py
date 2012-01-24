'''
Created on Jul 5, 2010

@author: root
'''
from PyQt4 import QtGui, QtCore
from CDUtilsPack.MetaUtils import Property
from CDUtilsPack.Telnet.TelnetClient import TelnetClient, TelnetParams
import os, threading

from PackUtils.CoreTrace import CoreTrace
from DRIDomain.TelnetClientDri import TelnetClientDri, TelnetTraceStates
from DRIDomain.TelnetClientDriAsync import TelnetClientDriAsync
from QConfigMain.CoreLogger import CoreLogger, CoreLogRecordProcessor
from PackUtils.CorePathes import TempFiles, IconFiles
from PackUtils.CoreCmdLineParams import CoreCmdLineParams
from DRIDomain.CoreIpsiusRunner import CoreIpsiusRunner
from QConfigMain.CoreLogStreams import PrintLogStream, FileLogStream
from QConfigMain.GuiDeviceCommon import GuiDeviceCommon
from QConfigMain.GuiDeviceList import GuiDeviceList
from QConfigMain.GuiSettings import GuiSettings
from QConfigMain.GuiLogStreams import ListWidgetLogStream

# ------------------------------------------------------------------

class GuiMainWidget(QtGui.QMainWindow):
    """Main window wrapper."""
    
    # using as filter-type in PrintLogStream
    class __PrintedLogOwner: pass
    
    def __init__(self, sett : CoreCmdLineParams, 
                 log : CoreLogger, 
                 domainName : str, telnetParams : TelnetParams,
                 errorHandler): # can raise Exception
        """
        'sett'         - command line parameters, 
        'log'          - main log class, using to register different output streams,  
        'domainName'   - ProjIpsius domain name,
        'telnetParams' - parameters that using to connect to the ProjIpsius,
        'errorHandler' - fn(e : Exception) -> None, using to handle all exceptions.
        """
        QtGui.QMainWindow.__init__(self)
        pyUiModule = TempFiles.UiPyModule('QConfigMain', 'Main')
        self.ui = pyUiModule.Ui_MainWindow()
        self.ui.setupUi(self)
        
        self.errorHandler = errorHandler
        self.log = log
        self.guiIpsiusLogStream = None # ListWidgetLogStream
        self.commonLogTypesMap = None  # {type : widget}
        self.commonLogStream = None    # ListWidgetLogStream
        self.dri = None                # TelnetClientDriAsync
        self.devListWrapper = None     # GuiDeviceList
        self.devListUpdCounter = 0  
        self.startupConf = GuiSettings()  
        self.threadId = threading.current_thread()
        
        self.closed = False
        self.ipsiusUnexpClosed = False # to show err-box from closeEvent
        
        self.__SetupLog(sett)
        self.__SetupUi(sett) # after log setup!
        self.__InitDri(domainName, telnetParams)
        self.__InitDevListWrapper(sett)         
    
    
    def closeEvent(self, event : QtGui.QCloseEvent): # override
        if self.closed: return 
        
        self.__Log("closeEvent: started")
        
        # close DRI session
        if self.dri: 
            self.dri.Close()
            self.dri = None
        
        msg = None
        if self.ipsiusUnexpClosed:
            msg = "ProjIpsius was unexpectedly closed from outside or crashed."
            self.__LogAndPrint(msg)
            
        # ask save config changes
        if (self.devListWrapper and not self.devListWrapper.TryClose(msg)):
            self.ipsiusUnexpClosed = False
            self.__Log("closeEvent: aborted - need save changes")
            event.ignore()
            return
        
        self.closed = True
        
        self.__SaveConfigOnClose()
        self.__Log("closeEvent: completed.")
        self.__LogAndPrint("ProjIpsius closed.")
        event.accept()   


    def __SaveConfigOnClose(self):
        s = self.startupConf
        s.MainWidth = self.width() 
        s.MainHeight = self.height() 
        s.MainY = self.y()
        s.MainX = self.x()
        s.IsMaximized = self.isMaximized()
        s.Save()
        self.__LogAndPrint("Custom GUI settings saved.")

    
    def __SetupLog(self, sett : CoreCmdLineParams):
        IncludeFilter = CoreLogRecordProcessor.CreateIncludeFilterFn
        Format = CoreLogRecordProcessor.CreateFormatFn
        
        Add = self.log.AddOutput 
        
        # log print stream for GuiMainWidget
        Add(PrintLogStream(IncludeFilter([GuiMainWidget.__PrintedLogOwner]), 
                           Format("{text}")))
        # log widget stream for ipsius output 
        self.guiIpsiusLogStream = \
            ListWidgetLogStream(self.ui.logIpsius, IncludeFilter([CoreIpsiusRunner]))
        Add(self.guiIpsiusLogStream)
        
        self.commonLogTypesMap = {
                TelnetClient         : self.ui.logTelnet, 
                TelnetClientDri      : self.ui.logDri,
                TelnetClientDriAsync : self.ui.logDriAsync,
                GuiDeviceCommon      : self.ui.logDevice,
                GuiDeviceList        : self.ui.logDevList,
                GuiMainWidget        : self.ui.logMain}
                
        # log widget stream for selected classes from map --^
        self.commonLogStream = \
            ListWidgetLogStream(self.ui.log, self.__CommonLogFilterFn())
        Add(self.commonLogStream)
        
        self.__LogOuterSig.connect(self.__LogOuter)
    
    
    def __SetupUiLog(self, sett : CoreCmdLineParams):
        
        def ChangeFilter():
            self.commonLogStream.UpdateFilter(self.__CommonLogFilterFn())
        
        self.ui.logMain.stateChanged.connect(ChangeFilter)
        self.ui.logDri.stateChanged.connect(ChangeFilter)
        self.ui.logDriAsync.stateChanged.connect(ChangeFilter)
        self.ui.logTelnet.stateChanged.connect(ChangeFilter)
        self.ui.logDevList.stateChanged.connect(ChangeFilter)
        self.ui.logDevice.stateChanged.connect(ChangeFilter)
    
        self.ui.logs.hide()
        
        if sett.TraceToGui:
            self.ui.toolBar.show()
            self.ui.logs.show() 
            self.commonLogStream.ShowGui()
            self.guiIpsiusLogStream.ShowGui()
            
        def OnShowHideLog(tabIndex : int):
            if self.ui.tabLogIpsius.isVisible():
                self.guiIpsiusLogStream.ShowGui()
                self.commonLogStream.HideGui()
            elif self.ui.tabLog.isVisible():
                self.commonLogStream.ShowGui()
                self.guiIpsiusLogStream.HideGui()                            
        
        self.ui.tabWidget.currentChanged.connect(OnShowHideLog)
        
    
    def __SetupUiFromStartupConfig(self):
        # load config from file
        self.startupConf.Load()
        s = self.startupConf
        
        if s.IsMaximized: return
        
        # set geometry (geometry - form rectangle WITHOUT window title)
        dx = self.geometry().x() - self.x()
        dy = self.geometry().y() - self.y()
        geometry = QtCore.QRect(s.MainX - dx, s.MainY - dy, 
                                s.MainWidth, s.MainHeight)
        self.setGeometry(geometry)
        self.move(s.MainX, s.MainY)
        #self.ui.devices.setWidth(s.DeviceListWidth)  
    

    def __SetupToolBar(self):
        
        def AddAct(name : str, slot, checkable = False):
            act = QtGui.QAction(self.ui.toolBar)
            act.setText(act.tr(name))
            act.triggered.connect(slot)
            act.setCheckable(checkable);
            act.setChecked(False);
            self.ui.toolBar.addAction(act)
            self.ui.toolBar.addSeparator()
            #self.toolBarActs[name] = act
        
        def OnSettings(): pass
    
        def OnShowHideLog():
            if self.ui.logs.isVisible():
                self.ui.logs.hide()  
                self.commonLogStream.HideGui()
                self.guiIpsiusLogStream.HideGui()          
            else:
                self.ui.logs.show() 
                self.commonLogStream.ShowGui()
                self.guiIpsiusLogStream.ShowGui()        
            
        #AddAct("Settings", OnSettings)
        AddAct("Log",      OnShowHideLog, True)
        #AddAct("Exit",     self.close)
        
        self.ui.toolBar.hide()
    
    
    def __SetupUi(self, sett : CoreCmdLineParams):
        
        def SetIcon():
            assert os.path.exists(IconFiles.Main) 
            self.setWindowIcon(QtGui.QIcon(IconFiles.Main))
        
        SetIcon()
        self.__SetupToolBar()
        self.__SetupUiLog(sett)
        self.__SetupUiFromStartupConfig()
    
    
    def __InitDri(self, domainName : str, params : TelnetParams):
        self.__LogAndPrint("Connecting via Telnet ...")
        assert not self.dri
        
        self.dri = TelnetClientDriAsync(domainName, params, 
                                        self.errorHandler,
                                        CoreTrace(self.__LogOuter, True))
        
        def EnableLog():
            states = TelnetTraceStates(False)
            states.ForDri = self.ui.logDri.isChecked()
            states.ForDriAsync = self.ui.logDriAsync.isChecked()
            states.ForTelnet = self.ui.logTelnet.isChecked()
            self.dri.EnableTrace(states)
            
        self.ui.logDri.stateChanged.connect(EnableLog)
        self.ui.logDriAsync.stateChanged.connect(EnableLog)
        self.ui.logTelnet.stateChanged.connect(EnableLog)
        
        EnableLog()
        
        self.__LogAndPrint("Connected.")
    
    
    def __InitDevListWrapper(self, sett : CoreCmdLineParams):
        assert not self.devListWrapper and self.dri
        self.devListWrapper = \
            GuiDeviceList(self.ui.deviceList, self.dri, 
                          sett.ConfigDir, self.ui.deviceArea, 
                          self.ui.statusbar.showMessage,
                          self.errorHandler,
                          CoreTrace(self.__LogOuter, True))
        
        def EnableTrace():
            self.devListWrapper.EnableTrace(self.ui.logDevList.isChecked(),
                                            self.ui.logDevice.isChecked())
        
        self.ui.logDevList.stateChanged.connect(EnableTrace)
        self.ui.logDevice.stateChanged.connect(EnableTrace)
        
        EnableTrace()
        
        if self.startupConf.IsMaximized: self.showMaximized()
        else: self.show()
        
    
    def __CommonLogFilterFn(self): 
        """
        Return fn(record) -> bool
        """
        assert self.commonLogTypesMap
        
        useList = [GuiMainWidget.__PrintedLogOwner]
        for t in self.commonLogTypesMap.keys():
            if self.commonLogTypesMap[t].isChecked():
                useList.append(t)
        
        if useList:
            return CoreLogRecordProcessor.CreateIncludeFilterFn(useList)
        else:
            return CoreLogRecordProcessor.CreateIncludeAllFilterFn() 
    

    __LogOuterSig = QtCore.pyqtSignal(QtCore.pyqtWrapperType, 
                                      QtCore.pyqtWrapperType,
                                      QtCore.pyqtWrapperType)

    
    def __LogOuter(self, obj, text : str, textID : str = None):
        if self.threadId == threading.current_thread():
            self.log.Add(obj, text, textID)
            return        
        
        self.__LogOuterSig.emit(obj, text, textID)
    
       
    def __Log(self, text : str):
        self.log.Add(self, text)       
         
    
    def __LogAndPrint(self, text : str):
        self.log.Add(GuiMainWidget.__PrintedLogOwner(), text)       
    
    
    IpsiusUnexpectedlyClosed = Property('ipsiusUnexpClosed', True)
    
    
    
