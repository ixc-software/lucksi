'''
Created on Mar 22, 2010

@author: root
'''

from PyQt4 import QtCore, QtGui

from DRIDomain.TelnetClientDri import DriError
from QConfigMain.CoreDeviceBroadcastInfo import (CoreDeviceBroadcastInfo, 
                                          CoreDeviceBroadcastInfoList)
from QConfigMain.GuiDeviceConfig import GuiDeviceConfig, GuiDeviceConfigHandler
from QConfigMain.GuiDevice import GuiDevice
from QConfigMain.GuiDeviceFactory import GuiDeviceFactory, GuiDeviceFactoryErr
from PackUtils.CoreTrace import CoreTrace
from DRIDomain.TelnetClientDriAsync import CommandDriHandler, TelnetClientDriAsync
from DRIDomain.TelnetClientDriAsyncQueue import TelnetClientDriAsyncQueue


# --------------------------------------------------------

_CRequestBroadcastIntervalMs = 5 * 1000
_CUpdateStatusIntervalMs     = 500

_CBroadcastCmd = "HwFinder.ListBoards false"

_CBroadcastStatus = '[ Updated from broadcast: {0} ]'
_CDeviceStatus    = '[ {0}: {1} ]'

# --------------------------------------------------------

class GuiDeviceList(QtCore.QObject):
    """
    List of all devices. Using QListWidget to display list of devices.
    Create devices from config files stored on disc and from broadcast info.
    Update existed devices from broadcast info.
    """
    
    def __init__(self, 
                 widget : QtGui.QListWidget, 
                 dri : TelnetClientDriAsync,
                 devConfigDir : str, devParentWidget : QtGui.QScrollArea, 
                 statusFn, errorHandler, trace : CoreTrace):
        """
        'widget'           - widget to display device list,
        'devConfigDir', 
        'devParentWidget', 
        'devDri'           - see GuiDevice,
                             using to display results of broadcast commands,
        'statusFn'         - fn(text : str) -> None, using to display device 
                             status,
        'errorHandler'     - fn(e : Exception) -> None.
        """
        QtCore.QObject.__init__(self)
        assert widget != None and devParentWidget != None
        self.widget = widget
        self.widget.setSortingEnabled(True)
        self.runningAct = None # QtGui.QAction
        self.deleteAct = None # QtGui.QAction
        
        self.errorHandler = errorHandler
        self.trace = trace
        self.statusFn = statusFn
        self.inClosingState = False
        
        self.updateStatusTimer = None # QtCore.QTimer
        self.updateBroadcastTimer = None # QtCore.QTimer
        self.broadcastCounter = 0
        
        self.devTypeFactory = GuiDeviceFactory() # can raise Exception
        self.devList = []
        self.devParentWidget = devParentWidget
        self.devConfigDir = devConfigDir
        self.currDevStaticName = None # no device's widget is displayed
        self.currDeviceStatus = ''
        self.dri = TelnetClientDriAsyncQueue(dri, self.errorHandler)
        
        self.__SetupUi()
        self.__InitTimers() #logFn)
        
        def LoadConfigs():
            confDictList = GuiDeviceConfigHandler.ReadAllFiles(self.devConfigDir)
            for confDict in confDictList:
                self.__AddItem(None, confDict) 
        
        LoadConfigs()
    
    
    def __len__(self) -> int:
        assert(self.widget.count() == len(self.devList))
        return self.widget.count()
    
    
    def __AddItem(self, info : CoreDeviceBroadcastInfo, confDict : {}):
            
            def DeviceNameChecker(newName : str, staticName : str) -> bool:
                for dev in self.devList:
                    if dev.StaticName != staticName and dev.Name == newName:
                        return True
                return False    
            
            def DeviceStatus(name : str, text : str):
                self.__Status(text) # use current device name 
       
            self.__Trace("Add item.")    
            if info: staticName = info.StaticName
            else: 
                assert confDict
                staticName = confDict['devStaticName'] 
            
            assert not self.__FindDevice(staticName)
            
            dev = GuiDevice(DeviceNameChecker, info, confDict, 
                            self.devConfigDir, self.dri, self.devTypeFactory,
                            DeviceStatus, self.errorHandler, self.trace.Copy())
            self.devList.append(dev)
            item = QtGui.QListWidgetItem(dev.ShortDescription())
            item.setData(QtCore.Qt.UserRole, dev.StaticName) 
            self.widget.addItem(item)
    
    
    def __Trace(self, text : str):
        self.trace.Add(self, text)
    
    
    def __HasDisplayedDevice(self):
        """
        Return True if any device widget is displayed.
        """
        if self.currDevStaticName == None: return False
        for dev in self.devList:
            if self.currDevStaticName == dev.StaticName:
                return True
        return False
    
    
    def __FindDevice(self, staticName : str) -> GuiDevice or None:
        if staticName == None: return
        
        for i in range(self.__len__()):
            if self.devList[i].StaticName == staticName:
                return self.devList[i]             
    
    
    def __FindCurrDevice(self) -> GuiDevice:
        assert self.__HasDisplayedDevice()
        dev = self.__FindDevice(self.currDevStaticName)
        assert dev
        return dev
            
    
    def __FindDeviceRow(self, staticName : str) -> int:
        assert staticName
        for i in range(self.widget.count()):
            if self.widget.item(i).data(QtCore.Qt.UserRole) == staticName:
                return i
        raise AssertionError("Item with StaticName = '" + staticName + 
                             "' was not found in QListWidget")
    
    
    def __FindCurrDeviceRow(self) -> int:
        return self.__FindDeviceRow(self.currDevStaticName)
    
    
    def __OnItemClicked(self, item : QtGui.QListWidgetItem):
            
        def TryHideCurrentDevice() -> bool:
            if not self.__HasDisplayedDevice(): 
                return True
            if not self.__FindCurrDevice().TryHide(self.devParentWidget): 
                self.widget.setCurrentRow(self.__FindCurrDeviceRow())
                return False
            return True
                    
        if self.inClosingState or not TryHideCurrentDevice(): return            
        
        self.currDevStaticName = self.widget.currentItem().data(QtCore.Qt.UserRole)
        currDev = self.__FindCurrDevice()
        
        def OnChanged():
            widget = self.widget.item(self.__FindCurrDeviceRow())
            if not widget: return # ignore callbacks from deleted items
            widget.setText(currDev.ShortDescription())
            
        currDev.DescChangedSig.connect(OnChanged)
        currDev.Show(self.devParentWidget)
                
    
    def __OnMenu(self, pos : QtCore.QPoint):
        if self.inClosingState or self.__len__() == 0: return 
                
        dev = self.__FindCurrDevice()
        # modify action for current item
        self.runningAct.setChecked(dev.IsRunning());
        menu = QtGui.QMenu(self.widget)
        if not dev.HasUnknownType: 
            menu.addAction(self.runningAct);
            menu.addSeparator();
        menu.addAction(self.deleteAct);
        menu.exec(self.widget.cursor().pos());
    
    
    def __OnRunStopCurrDevice(self, checked : bool):
        dev = self.__FindCurrDevice()
        if dev.IsRunning(): dev.Stop()
        else: dev.Run()
    
    
    def __OnDeleteCurrDevice(self, checked : bool):
        itemIndex = self.widget.currentRow()
        itemStaticName = self.widget.item(itemIndex).data(QtCore.Qt.UserRole)
        dev = self.__FindDevice(itemStaticName)
        
        def AreYouSure() -> QtGui.QMessageBox.StandardButton:
            msg = "Delete device '{}'?".format(dev.Name)
            return  QtGui.QMessageBox.question(
                            self.widget, "Device List", msg,
                            QtGui.QMessageBox.Ok, QtGui.QMessageBox.Cancel)
        
        if AreYouSure() == QtGui.QMessageBox.Cancel:
            return
            
        self.__Trace("Delete item: '{0}'".format(dev.Name))
        self.currDeviceStatus = ''
        if itemStaticName == self.currDevStaticName: 
            self.currDevStaticName = None
        item = self.widget.takeItem(itemIndex)
        item = None
        
        widget = self.devParentWidget.takeWidget()
        widget = None                
        
        dev.DeleteConfig()        
        self.devList.remove(dev)
        assert(self.widget.count() == len(self.devList))
        
        
    def __SetupUi(self):
        self.widget.itemClicked.connect(self.__OnItemClicked)
        
        self.widget.setContextMenuPolicy(QtCore.Qt.CustomContextMenu)
        self.widget.customContextMenuRequested.connect(self.__OnMenu)
        
        self.runningAct = QtGui.QAction(self)
        self.runningAct.setText(self.runningAct.tr("Running")) 
        self.runningAct.triggered.connect(self.__OnRunStopCurrDevice)
        self.runningAct.setCheckable(True)
        self.runningAct.setChecked(True)
        
        self.deleteAct = QtGui.QAction(self)
        self.deleteAct.setText(self.deleteAct.tr("Delete"))
        self.deleteAct.triggered.connect(self.__OnDeleteCurrDevice)
    
    
    def __OnBroadcast(self):
            
        def UpdCallback(res : CommandDriHandler):
            if res.IsAborted: return
            
            self.broadcastCounter += 1
            self.__Trace("Executed: {0}\nResult:{1}".format(_CBroadcastCmd, res))
            #logFn(self, str(res))
            if not res.ResultsOK:
                self.errorHandler(DriError(res.LastResult, _CBroadcastCmd)) 
                return
            
            assert len(res.Results) == 1                 
                
            devInfoList = CoreDeviceBroadcastInfoList(res.Results[0].Text)
            self.__Trace("Updated from broadcast.")
            self.__Trace("Parsed device list:")
            for devInfo in devInfoList:
                self.__Trace("{0}: {1}".format(devInfo.Name, devInfo.FullDesc()))
                dev = self.__FindDevice(devInfo.StaticName)
                if not dev:
                    self.__AddItem(devInfo, None)
                    continue
                dev.UpdateBroadcastInfo(devInfo)
                item = self.widget.item(self.__FindDeviceRow(devInfo.StaticName)) 
                item.setText(dev.ShortDescription())
        
        if self.inClosingState: 
            self.updateBroadcastTimer.stop()
            return
        
        self.dri.Process(_CBroadcastCmd, UpdCallback)
    
    
    def __InitTimers(self): 
        OnUpdateStatus = lambda : self.__Status()
        
        self.updateStatusTimer = QtCore.QTimer(self)
        self.updateStatusTimer.timeout.connect(OnUpdateStatus)
        self.updateStatusTimer.start(_CUpdateStatusIntervalMs)
        
        
    
        self.updateBroadcastTimer = QtCore.QTimer(self)
        self.updateBroadcastTimer.timeout.connect(self.__OnBroadcast)
        self.updateBroadcastTimer.start(_CRequestBroadcastIntervalMs)
    
    
    def __Status(self, deviceStatus : str = None):
        msg = _CBroadcastStatus.format(self.broadcastCounter)
        dev = self.__FindDevice(self.currDevStaticName)
        devName = dev.Name if dev else None
        if devName and deviceStatus: 
            self.currDeviceStatus = _CDeviceStatus.format(devName, deviceStatus)
        
        if self.currDeviceStatus:
            msg += self.currDeviceStatus 

        self.statusFn(msg)
    
    
    def TryClose(self, whyCloseInfo : str = None) -> bool:
        """
        'areYouSureMsg' - additional info that showing in device 'Save config?' 
                          pop-up message.
        Prevent self of using DRI more. Trying to close current device.
        """
        
        def TryCloseCurrentDevice() -> bool:
            if not self.__HasDisplayedDevice(): 
                if whyCloseInfo:
                    QtGui.QMessageBox.warning(self.widget, "Error", whyCloseInfo, 
                                              QtGui.QMessageBox.Ok)
                return True
            if not self.__FindCurrDevice().TryClose(self.devParentWidget,
                                                    whyCloseInfo): 
                return False
            return True
        
        self.inClosingState = True
        res = TryCloseCurrentDevice()
        if res: 
            self.dri.AbortAll()
            self.dri = None
        
        return res
    
    
    def EnableTrace(self, forSelf : bool, forDevice : bool):
        self.trace.Enable(forSelf)
        for dev in self.devList:
            dev.EnableTrace(forDevice)
    
# -------------------------------------------------------------
# Test
# -------------------------------------------------------------

from TestDecor import UtTest
from AllTest.TestRunner import GTestRunner
from AllTest.TestParams import TestParams
from PackUtils.CorePathes import TempFiles

        
@UtTest
def GuiDeviceListTest():
    
    CTraceTest = False
    CShowWidget = CTraceTest
    
    def Impl(p : TestParams, 
             parentLeftWidget : QtGui.QWidget, 
             parentRightWidget : QtGui.QWidget):
    
        dl = GuiDeviceList(parentLeftWidget, p.DRI,
                           TempFiles.DirTest, 
                           parentRightWidget, 
                           lambda text: p.Output(text), 
                           lambda e: p.ErrHandler(e), 
                           p.CreateCoreTrace(CTraceTest))
        
        dl.EnableTrace(False, True)
        assert dl.TryClose()
        p.Complete(closeApp = not CShowWidget)
        
            
    GTestRunner.RunGuiMainIpsiusDri(Impl, CShowWidget, QtGui.QListWidget, QtGui.QScrollArea, 
                                    CTraceTest, enableDriTrace = False, startEventLoop = True)

# -------------------------------------------------------------
    
if __name__ == "__main__":
    
    import unittest
    unittest.main()
    
    