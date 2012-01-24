'''
Created on Jul 15, 2010

@author: root
'''

from PyQt4 import QtGui, QtCore
from TestDecor import UtTest
from CDUtilsPack.MiscUtils import AssertForException

from PackUtils.Misc import CheckFnArgs
from CDUtilsPack.Telnet.TelnetParams import TelnetParams
from DRIDomain.TelnetClientDriAsync import TelnetClientDriAsync, CommandDriHandler
from PackUtils.CoreTrace import CoreTrace
from AllTest.TestParams import TestParams
from DRIDomainTest.TestIpsiusRunner import TestIpsiusRunner


class TestGuiMainIpsiusRunner(QtGui.QMainWindow):
    
    def __init__(self, showWidget : bool, 
                 parentLeftWidgetType, parentRightWidgetType):
        """
        'testFn' - fn(p : TestParams, parentLeftWidgetType, parentRightWidgetType) -> None
        or 
        'testFn' - fn(p : TestParams, parentWidgetType) -> None
        """
        QtGui.QMainWindow.__init__(self)
        self.closed = False
        self.resize(600, 600)
        
        
        assert parentLeftWidgetType
        left = QtGui.QDockWidget()
        left.setWidget(parentLeftWidgetType()) 
        self.addDockWidget(QtCore.Qt.LeftDockWidgetArea, left)
        self.lWidget = left.widget()
        self.rWidget = None
        
        if parentRightWidgetType:
            right = QtGui.QDockWidget()
            right.setWidget(parentRightWidgetType()) 
            self.addDockWidget(QtCore.Qt.RightDockWidgetArea, right)
            self.rWidget = right.widget()
        
        if showWidget: 
            self.show()
            self.lWidget.show()
            if self.rWidget: self.rWidget.show()
    
    
    def Run(self, testFn, outputFn, completeFn, withDri : bool, 
            enableDriTrace : bool, ipsiusTrace : CoreTrace, ipsiusExePath, udplogExePath):
                                
        def TestFnWrapper(p : TestParams):
            p.SetupDri(enableDriTrace)
            p.Output("Running: " + testFn.__name__)
            if self.rWidget: testFn(p, self.lWidget, self.rWidget)
            else: testFn(p, self.lWidget)
            
        def CloseFn(exitCode : int):
            self.lWidget.close()
            if self.rWidget: self.rWidget.close()
            self.closed = True
            self.close()
            completeFn(exitCode)
            
        self.runner = TestIpsiusRunner(TestFnWrapper, outputFn, CloseFn, withDri, 
                                       enableDriTrace, ipsiusTrace, 
                                       ipsiusExePath, udplogExePath)
        
            
    def closeEvent(self, event : QtGui.QCloseEvent):
        if self.closed: return
        self.closed = True
        self.runner.Close()   
                        
        
   