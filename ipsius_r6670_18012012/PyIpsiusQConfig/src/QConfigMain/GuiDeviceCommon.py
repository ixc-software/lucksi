'''
Created on Apr 16, 2010

@author: root
'''

from PyQt4 import QtGui
from PackUtils.Misc import CheckFnArgs
from PackUtils.CoreTrace import CoreTrace

# -------------------------------------------------------

class GuiDeviceCommon:
    def __init__(self, staticName : str, statusFn, trace : CoreTrace):
        """
        'staticName'  - nonchangeable name of device,
        'statusFn'    - fn(name : str, text : str) -> None, 
                        using to show device status.
        """
        self.staticName = staticName
        self.onRunWidgetStateList = [] # (QtGui.QWidget, bool)
        self.onStopWidgetStateList = [] # (QtGui.QWidget, bool)
        self.statusFn = statusFn
        self.trace = trace
        
        CheckFnArgs(self.statusFn, (str, str), None)
        assert isinstance(trace, CoreTrace)
    
    def EnableTrace(self, state : bool):
        self.trace.Enable(state)
    
    def Trace(self, text : str):
        self.trace.Add(self, text, self.staticName)
    
    def Status(self, text : str):
        self.statusFn(self.staticName, text)
    
    def EnableWidget(self, widget : QtGui.QWidget, 
                     onRun : bool, onStop : bool):
        """Only widgets with setEnabled() method is accepted."""
        assert widget
        self.onRunWidgetStateList.append((widget, onRun))
        self.onStopWidgetStateList.append((widget, onStop))    
    
    def RunStarted(self):
        """Called after run script started."""
        for widgetState in self.onRunWidgetStateList:
            widgetState[0].setEnabled(widgetState[1])
    
    def StopCompleted(self):
        """Called after stop script completed."""
        for widgetState in self.onStopWidgetStateList:
            widgetState[0].setEnabled(widgetState[1])
            
# -------------------------------------------------------
# Tests
# -------------------------------------------------------

from TestDecor import UtTest
from AllTest.TestRunner import GTestRunner
from AllTest.TestParams import TestParams



@UtTest
def GuiDeviceCommonTest():
    
    def Impl(p : TestParams):
        
        statusList = []
        traceList = []
        
        def StatusFn(name : str, text : str): 
            statusList.append(text)
            
        def TraceFn(obj, text : str, textID : str): 
            traceList.append(text)
        
        c = GuiDeviceCommon('x', StatusFn, CoreTrace(TraceFn, True))
        c.Status('st')
        p.Output('status: ' + statusList[0])
        assert statusList[0] == 'st'
        c.Trace('de')
        p.Output('trace: {}'.format(traceList[0]))
        assert traceList[0] == 'de'
        
        w = QtGui.QLineEdit('x', None)
        assert w.isEnabled() == True
        c.EnableWidget(w, False, True)
        
        c.RunStarted()
        assert w.isEnabled() == False
        
        c.StopCompleted()
        assert w.isEnabled() == True 
        
        c.EnableTrace(False)
        
        p.Output("GuiDeviceCommonTest: OK")
        p.Complete()
        
    GTestRunner.RunApp(Impl, traceTest = False)
    
# -------------------------------------------------------

if __name__ == "__main__":
    import unittest
    unittest.main()

    

    
    
    
    