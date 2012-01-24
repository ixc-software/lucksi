'''
Created on Jun 30, 2010

@author: root
'''

from PyQt4 import QtGui
from QConfigMain.CoreLogger import (CoreLogger, CoreLogStore, CoreLogRecord, 
                             IBaseLogStream, CoreLogRecordProcessor)
from PackUtils.Misc import CheckFnArgs
import threading


class ListWidgetLogStream(IBaseLogStream):
    """
    Show log in widget if only it is visible. 
    """
    
    def __init__(self, guiList : QtGui.QListWidget, filterFn, 
                 formatFn = CoreLogRecordProcessor.CreateFormatFn(),  
                 dropSize : int = 500):
        """
        'formatFn' - fn(record) -> str, create it using CoreLogger.CreateFormatFn(),
        'filterFn' - fn(record) -> bool, create it using one of the CoreLogger 
                     create filter methods,
        'guiList'  - widget to display log,
        'dropSize' - log will be dropped when it size reach this value 
                     (0 and -1 - infinity).
        """
        assert guiList != None
        CheckFnArgs(filterFn, (CoreLogRecord,), bool)
        CheckFnArgs(formatFn, (CoreLogRecord,), str)
        
        self.gui = guiList
        self.dontDisplayInGuiIfHidden = True
        
        OnDrop = lambda: guiList.clear()
        self.storage = CoreLogStore(OnDrop, dropSize)
        
        self.filterFn = filterFn
        self.formatFn = formatFn
        RP = CoreLogRecordProcessor
        self.outputFn = RP.CreateOutputFn(self.gui.addItem, 
                                          RP.OutputFormat.SplitLinesSkipEmpty)
        
        self.threadId = threading.current_thread()  
        self.closed = False  
    
    
    def __del__(self):
        assert self.closed     
    
    
    def __Check(self):
        assert not self.closed
        assert self.threadId == threading.current_thread()
                        
            
    def Add(self, record : CoreLogRecord): # IBaseLogStream impl
        """
        Add to storage filtered records only.
        """
        self.__Check()
        
        if not self.filterFn(record):
            return
        
        self.storage.Add(record)
        
        if not self.gui.isVisible() and self.dontDisplayInGuiIfHidden: 
            return
        
        # show in gui
        scroll = self.gui.verticalScrollBar()
        scrollWasAtEnd = (scroll.value() == scroll.maximum())
        
        if self.filterFn(record): self.outputFn(self.formatFn(record))
        if (scrollWasAtEnd): self.gui.scrollToBottom()
    
    
    def Close(self): # IBaseLogStream impl
        """Clear widget."""
        self.HideGui()
        self.closed = True
                      
    
    def UpdateFilter(self, filterFn):
        """
        'filterFn' - fn(record : CoreLogger.Record) -> bool
        """
        self.__Check()        
        assert filterFn
        self.filterFn = filterFn
    
    
    def ShowGui(self):
        """Copy log to widget."""
        self.__Check()        
        self.gui.show()
        CoreLogRecordProcessor.Process(self.storage, self.outputFn, self.filterFn, 
                                       self.formatFn)
        self.gui.scrollToBottom()
    
    
    def HideGui(self):
        """Clear widget."""
        self.__Check()        
        self.gui.clear()
        self.gui.hide()

# ---------------------------------------------------------
# Test
# ---------------------------------------------------------

from TestDecor import UtTest
from AllTest.TestRunner import GTestRunner
from AllTest.TestParams import TestParams

@UtTest
def ListWidgetLogStreamTest():
    
    def Impl(p : TestParams):
        owner1 = "x"
        owner2 = 5
        
        filterFn = CoreLogRecordProcessor.CreateIncludeFilterFn([str])
        
        l = CoreLogger()
        w = QtGui.QListWidget()
        st = ListWidgetLogStream(w, filterFn)
        l.AddOutput(st)
        
        p.Output("Add data while widget is invisible")
        w.hide()
        l.Add(owner1, "x")
        l.Add(owner1, "x2")
        l.Add(owner2, "x3") # ignored by filter
        assert w.count() == 0
        
        p.Output("Make widget visible, copy log to it")
        st.ShowGui()
        assert w.count() == 2
        
        p.Output("Add data while widget is visible")
        l.Add(owner2, "x4") # ignored by filter
        l.Add(owner1, "x5")
        l.Add(owner1, "x6")
        assert w.count() == 4
        
        p.Output("Hide widget, clear log in widget")
        st.HideGui()
        assert w.count() == 0
        
        p.Output("Make widget visible, don't copy log to it")
        w.show()
        l.Add(owner1, "x7")
        assert w.count() == 1
        
        p.Output("Close")
        l.Close() # close stream
        assert w.count() == 0
        
        p.Output("ListWidgetLogStreamTest: OK")
        p.Complete()
        

    GTestRunner.RunApp(Impl, traceTest = False)

# ---------------------------------------------------------

if __name__ == "__main__":
    
    import unittest
    unittest.main()

    
    
    
    
    
    
    
    