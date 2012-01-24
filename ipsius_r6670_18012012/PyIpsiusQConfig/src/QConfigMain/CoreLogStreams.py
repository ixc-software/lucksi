'''
Created on Jun 30, 2010

@author: root
'''
from PackUtils.CorePathes import TempFiles
from CDUtilsPack.MetaUtils import Property
from QConfigMain.CoreLogger import (CoreLogger, CoreLogRecord, CoreLogStore, 
                             IBaseLogStream, CoreLogRecordProcessor)
from PackUtils.Misc import CheckFnArgs
import time, os


# -------------------------------------------------------

class FileLogStream(IBaseLogStream):
    """
    Using to save log to file. Save to file each N records.
    """
    
    def __init__(self, fileName : str, saveEachCount : int = -1, 
                 filterFn = CoreLogRecordProcessor.CreateIncludeAllFilterFn(), 
                 formatFn = CoreLogRecordProcessor.CreateFormatFn()):
        """
        'fileName'      - using to generate unique file name to save log,
                          can't be empty,
        'formatFn'      - fn(res : Record) -> str, 
        'filterFn'      - fn(res : Record) -> bool,
        'saveEachCount' - saving log to file each 'saveEachCount' records,
                          if 0 or -1 - save only when Close() is called.
        """
        assert fileName
        CheckFnArgs(filterFn, (CoreLogRecord,), bool)
        CheckFnArgs(formatFn, (CoreLogRecord,), str)
                
        def FileName() -> str or None:
            dt = time.strftime("%Y.%m.%d_%H.%M.%S")
            file = "{0}_{1}.log".format(dt, fileName)
            return TempFiles.LogFile(file)
        
        self.name = FileName()
        self.storage = CoreLogStore(self.__Write, saveEachCount)
        self.formatFn = formatFn
        self.filterFn = filterFn
        self.closed = False
    
    
    def __del__(self):
        assert self.closed
    
    
    def __CheckNotClosed(self):
        assert not self.closed
    
    
    def __Write(self):
        assert self.storage
        
        RP = CoreLogRecordProcessor
        with open(self.name, 'a+t') as f:
        
            def Write(text : str):
                f.write(text)
                f.write('\n')
            
            Output = RP.CreateOutputFn(Write, RP.OutputFormat.DontSplitLines)
            
            # write all from storage, filter when add to it 
            RP.Process(self.storage, Output, formatFn = self.formatFn)
        
            
    def Add(self, record : CoreLogRecord): # IBaseLogStream impl
        """
        Add to storage filtered records only.
        """
        self.__CheckNotClosed()
        if self.filterFn(record): self.storage.Add(record)
    
    
    def Close(self): # IBaseLogStream impl
        self.__CheckNotClosed()
        
        self.closed = True
        self.__Write()
        self.storage.Clear()
        
    
    FileName = Property('name')
    
    
# -------------------------------------------------------

class PrintLogStream(IBaseLogStream):
    """
    Check each added record with filter and output via print() using 
    specified format.
    """
    
    def __init__(self, filterFn, formatFn):
        CheckFnArgs(filterFn, (CoreLogRecord,), bool)
        CheckFnArgs(formatFn, (CoreLogRecord,), str)
                
        self.filterFn = filterFn
        self.formatFn = formatFn
        self.closed = False
    
    
    def __del__(self):
        assert self.closed
    
    
    def __CheckNotClosed(self):
        assert not self.closed 
        
    
    def Add(self, record : CoreLogRecord): # IBaseLogStream impl
        self.__CheckNotClosed()
        if not self.filterFn(record): return
        print(self.formatFn(record))
    
    
    def Close(self): # IBaseLogStream impl
        self.__CheckNotClosed()
        self.closed = True

# ----------------------------------------------------
# Test
# ----------------------------------------------------

from TestDecor import UtTest
from AllTest.TestRunner import GTestRunner


@UtTest
def FileLogStreamTest():
    
    owner1 = "owner"
    owner2 = 5
    
    fileWriteCount = 2
    FormatFn = CoreLogRecordProcessor.CreateFormatFn('[{text}]')
    
    def CheckAndRemoveFile(name : str, correctData : [str]):
        data = ''
        with open(name, 'r') as file:
            data = file.read()
        
        assert data.splitlines() == correctData
        os.remove(name)

    def TestAddAndLogDrop():
        l = CoreLogger()
        st = FileLogStream("CoreLoggerTest1", fileWriteCount, formatFn = FormatFn)
        l.AddOutput(st)
        assert not os.path.exists(st.FileName)
        
        l.Add(owner1, "x")
        l.Add(owner1, "x2")
        l.Add(owner2, "x3") # drop log, write to file: x, x2
        assert os.path.exists(st.FileName)
        l.Add(owner2, "x4") 
        l.Add(owner2, "x5") # drop log, write to file: x3, x4
        CheckAndRemoveFile(st.FileName, ['[x]', '[x2]', '[x3]', '[x4]'])
        
        l.Close() # slose stream, write to file: x5
        assert os.path.exists(st.FileName)
        CheckAndRemoveFile(st.FileName, ['[x5]'])
               
    
    def TestCloseAndFilter():
        l = CoreLogger()
        st = FileLogStream("CoreLoggerTest2", fileWriteCount, 
                           CoreLogRecordProcessor.CreateIncludeFilterFn([str]), 
                           FormatFn)
        l.AddOutput(st)
        
        assert not os.path.exists(st.FileName)
        l.Add(owner1, "x")
        l.Add(owner2, "x2")
        assert not os.path.exists(st.FileName)
        l.Add(owner1, "x3")  
        l.Add(owner1, "x4") # drop log, write to file: x, x3
        CheckAndRemoveFile(st.FileName,  ['[x]', '[x3]'])
        
        l.Close() # write last
        CheckAndRemoveFile(st.FileName,  ['[x4]'])
    
    
    def Impl(outputFn):
        TestAddAndLogDrop()
        TestCloseAndFilter()
    
        outputFn("FileLogStreamTest: OK")
        
    GTestRunner.Run(Impl, traceTest = False)

# ----------------------------------------------------

if __name__ == "__main__":
    
    import unittest
    unittest.main()
