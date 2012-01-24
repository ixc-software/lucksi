'''
Created on Jun 2, 2010

@author: root
'''
from PyQt4 import QtCore

from CDUtilsPack.MetaUtils import Property
from abc import abstractmethod
from types import FunctionType
from CDUtilsPack.MiscUtils import AssertForException
import time


# ----------------------------------------------------

class CoreLogRecord:
    """
    Log record.
    """
    
    def __init__(self, obj, text : str, textID : str = None):
        """
        'obj'    - owner of the log message, using to get type and instance ID,
        'text'   - log message,
        'textID' - custom log message prefix: 
                   None - using type and instance ID,
                   '' or '<custom text>' - using just it,
        """
        self.text = text
        self.objID = hex(id(obj))
        self.objType = type(obj)
        self.textID = textID
        self.datetime = time.localtime()
                
    
    def __str__(self) -> str:
        return self.Format(self.text, "%Y.%m.%d %H:%M:%S.%q")
    
    
    def OwnerTypeMatch(self, type) -> bool:
        return issubclass(self.objType, type)
    
    
    def Format(self, recordFormat : str, dateTimeFormat : str) -> str:
        """
        'recordFormat'   - string-like format template with arguments: {date}, 
                           {text}, {textID}. Can't be empty. 
                           Example: "{textID}: {text}",
        'dateTimeFormat' - record date-time format in Python time format, also 
                           you can use '%q' to display milliseconds.
                           Can't be empty if 'recordFormat' contains {date}.  
                           Example: "%Y.%m.%d %H:%M:%S.%q" 
        """
        assert recordFormat
        if '{date}' in recordFormat: assert dateTimeFormat
        
        def TextID() -> str:
            if self.textID != None: return self.textID
            return "{0} ({1})".format(self.objType.__name__, self.objID)
        
        CMsFlag = '%q'
        
        # replace msec in format if any
        if CMsFlag in dateTimeFormat:
            t = time.clock()
            msec = int((t * 1000) % 1000)
            dateTimeFormat = dateTimeFormat.replace(CMsFlag, str(msec))        
        dt = time.strftime(dateTimeFormat, self.datetime)
        
        return recordFormat.format(date = dt, text = self.text, 
                                   textID = TextID())


# ----------------------------------------------------

class IBaseLogStream:
    """Base interface for all log stream classes."""

    @abstractmethod
    def Add(self, record : CoreLogRecord): pass
    
    @abstractmethod
    def Close(self): pass


# ----------------------------------------------------

class CoreLogStore:
    """
    Storage for log records. After size reach N records, call 'onDropFn' and 
    drop records. Using inside log streams to store data.
    """
        
    def __init__(self, onDropFn, dropSize : int = -1):
        """
        'dropSize' - log will be dropped when it size reach this value 
                     (0  - record is never added to storage, on each Add()
                           'onDropFn'is called; 
                      -1 - storage is never dropped, 
                      N  - storage drop N records before add (N + 1) record),     
        'onDropFn' - fn() -> None.
        """
        assert onDropFn
        self.records = [] # [CoreLogRecord]
        self.dropSize = dropSize
        self.onDropFn = onDropFn
    
    
    def __iter__(self):
        for r in self.records:
            yield(r)
    
    
    def __len__(self):
        return len(self.records)
    
    
    def Add(self, record : CoreLogRecord):
        if self.dropSize >= 0 and len(self) == self.dropSize: 
            self.onDropFn()
            if self.dropSize == 0: return
            self.Clear()
        
        self.records.append(record)
    
    
    def Clear(self):
        self.records = []
    

# ----------------------------------------------------

class CoreLogRecordProcessor:
    """
    Using to process records in CoreLogStore.
    
    Methods that create filters return:   fn(rec : CoreLogRecord) -> bool.
    Method that create formatter returns: fn(rec : CoreLogRecord) -> str.
    Method that create outputter returns: fn(text : str) -> None.        
    """
    
    __CDefaultRecordFormat   = "{date} [{textID}]: {text}"
    __CDefaultDatetimeFormat = "%Y.%m.%d %H:%M:%S.%q"
    
    __DefaultFilter = lambda record: True
    __DefaultFormat = lambda record: record.Format(
                            CoreLogRecordProcessor.__CDefaultRecordFormat, 
                            CoreLogRecordProcessor.__CDefaultDatetimeFormat)
    
    @staticmethod       
    def Process(log : CoreLogStore, outputFn, 
                filterFn = __DefaultFilter, formatFn = __DefaultFormat):
        """
        'outputFn' - fn(text : str) -> None, use CreateOutputFn() to create it,
        'formatFn' - fn(rec : CoreLogRecord) -> str, use CreateFormatFn() to 
                     create it,
        'filterFn' - fn(rec : CoreLogRecord) -> bool, use one of create filter 
                     methods to create it.
        """
        for rec in log:
            if not filterFn(rec): continue
            outputFn(formatFn(rec))
    
    
    @staticmethod
    def CreateFormatFn(recordFormat : str = __CDefaultRecordFormat,
                       recordDatetimeFormat : str = __CDefaultDatetimeFormat):
        """
        For parameters description see CoreLogRecord.Format().                           
        """
        def Fn(record : CoreLogRecord) -> str:
            return record.Format(recordFormat, recordDatetimeFormat)
        
        return Fn            
    
    
    @staticmethod
    def __CreateFilterFn(includeList : [], excludeList : []) -> FunctionType:
        
        def FilterFn(record : CoreLogRecord) -> bool:
            included = False
            excluded = False
            
            if includeList != None:
                for type in includeList:
                    if record.OwnerTypeMatch(type):
                        included = True
                        break
                if excludeList == None: return included
            
            if excludeList != None:
                for type in excludeList:
                    if record.OwnerTypeMatch(type):
                        excluded = True
                        break
                if includeList == None: return not excluded
            
            return included and not excluded
        
        return FilterFn
    
    
    @staticmethod
    def CreateIncludeAllFilterFn() -> FunctionType:
        return CoreLogRecordProcessor.__CreateFilterFn(None, [])
    
    @staticmethod
    def CreateExcludeAllFilterFn() -> FunctionType:
        return CoreLogRecordProcessor.__CreateFilterFn([], None)
    
    
    @staticmethod
    def CreateIncludeFilterFn(recordOwnerTypesList : []) -> FunctionType:
        """
        'recordOwnerTypesList' - list of record's owner types.
        """
        return CoreLogRecordProcessor.__CreateFilterFn(recordOwnerTypesList, None)
        
    
    @staticmethod
    def CreateExcludeFilterFn(recordOwnerTypesList : []) -> FunctionType:
        """
        'recordOwnerTypesList' - list of record's owner types.
        """
        return CoreLogRecordProcessor.__CreateFilterFn(None, recordOwnerTypesList)
    
    
    class OutputFormat:
        DontSplitLines = 0
        SplitLines = 1
        SplitLinesSkipEmpty = 2        
    
    
    @staticmethod
    def CreateOutputFn(outputFn, 
                       format = OutputFormat.DontSplitLines) -> FunctionType:
        """
        'outputFn' - fn(text : str) -> None.
        """
        
        def OutputFn(text : str):
            Format = CoreLogRecordProcessor.OutputFormat
            
            if format == Format.DontSplitLines:
                outputFn(text)
                return
        
            res = text.splitlines()
            for line in res:
                if not line and format == Format.SplitLinesSkipEmpty: 
                    continue
                outputFn(line)
        
        return OutputFn    

# ----------------------------------------------------

class CoreLogger:
    """Wrapper for list of log streams."""
    
    def __init__(self):
        self.streams = [] # list of classes like LogStreamSample
        self.closed = False
    
    
    def __del__(self):
        assert self.closed
    
    
    def __CheckNotClosed(self):
        assert not self.closed
        
        
    def Add(self, obj, text : str, textID : str = None):
        """
        'obj'    - owner of the log message,
        'text'   - log message,
        'textID' - custom log message prefix: 
                   None - using type and instance ID,
                   '' or '<custom text>' - using just it.
        """        
        self.__CheckNotClosed()

        rec = CoreLogRecord(obj, text, textID)
        for st in self.streams: 
            st.Add(rec)
            
    
    def AddOutput(self, stream : IBaseLogStream):
        """
        Check that streams isn't in list already. Add streams to list.
        """
        self.__CheckNotClosed()
        
        assert not stream in self.streams
        self.streams.append(stream)
    
    
    def RemoveOutput(self, stream : IBaseLogStream):
        """
        Check that streams is in list. Close() streams and remove them from list.
        """
        self.__CheckNotClosed()
        
        assert stream in self.streams
        stream.Close()
        self.streams.remove(stream)
    
    
    def Close(self):
        """Close() all streams. Clear streams list."""
        self.__CheckNotClosed()
        
        while self.streams:
            self.RemoveOutput(self.streams[0])
        
        self.closed = True
    
    
    @property
    def OutputStreamCount(self) -> int:
        return len(self.streams) 
    
# ----------------------------------------------------
# Tests
# ----------------------------------------------------

from TestDecor import UtTest
from AllTest.TestRunner import GTestRunner


@UtTest
def CoreLogStoreTest():
    
    def Add(s : CoreLogStore, recordOwner, text):
        s.Add(CoreLogRecord(recordOwner, text))
    
    RP = CoreLogRecordProcessor
    
    owner1 = 1
    owner2 = "owner2"
    
    def TestAdd(outputFn):
        def OnDropFn(): pass
        
        s = CoreLogStore(OnDropFn)
        Add(s, owner1, "x")
        Add(s, owner2, "x2")
        assert len(s) == 2        
        
    def TestDropAndClear(outputFn):
        dropCalls = []
        
        def OnDropFn():
            dropCalls.append(1)
        
        # test with > 0
        s = CoreLogStore(OnDropFn, 2)
        Add(s, owner1, "x")
        Add(s, owner1, "x")
        assert len(s) == 2
        Add(s, owner1, "x") # drop two records, add one
        assert len(s) == 1
        assert len(dropCalls) == 1
        s.Clear()
        assert len(s) == 0
        assert len(dropCalls) == 1
        dropCalls = []
        
        # test with 0
        s = CoreLogStore(OnDropFn, 0)
        Add(s, owner1, "x")
        assert len(s) == 0
        assert len(dropCalls) == 1
        dropCalls = []
        
        # test with -1
        s = CoreLogStore(OnDropFn, -1)
        Add(s, owner1, "x")
        Add(s, owner1, "x2")
        assert len(s) == 2
        assert len(dropCalls) == 0
        dropCalls = []
        
    def TestProcess(outputFn):
        def OnDropFn(): pass
        
        s = CoreLogStore(OnDropFn)
        Add(s, owner1, "x")
        Add(s, owner2, "x2")
        Add(s, owner1, "x3")
        
        results = []
        outputFn = RP.CreateOutputFn(results.append, RP.OutputFormat.DontSplitLines)
        formatFn = RP.CreateFormatFn('[{text}]')
        filterFn = RP.CreateIncludeFilterFn([type(owner2)])
        
        RP.Process(s, outputFn, filterFn, formatFn)
        assert (len(results) == 1 and results[0] == "[x2]") 
    
    def Impl(outputFn):
        TestAdd(outputFn)
        TestDropAndClear(outputFn)
        TestProcess(outputFn)
    
    GTestRunner.Run(Impl, traceTest = False)
        

# ----------------------------------------------------

_CTestResult = """TestClose
Add: 0
Add: 0
Close
[ObjX (0x8504f0c): 0]
Close
[ObjX (0x8504f0c): 0]
TestAddRemoveStreams
Close
Close
TestFormat
Format without dt
Add: 0
Close
[ObjX (0x8504f0c): 0]
Format with dt
Add: 0
Close
<10.06.30 15:24:40> [ObjX (0x8504f0c): 0]
TestFilter
Filter: include [ObjX, ObjY], split lines, skip empty
Close
[custom: 0: line1
line3]
[: 1]
[ObjY (0x8504f2c): 2]
Filter: include [ObjX, ObjY], split lines, don't skip empty
Close
[custom: 0: line1

line3]
[: 1]
[ObjY (0x8504f2c): 2]
Filter: include [ObjX, ObjY], don't split lines
Close
[custom: 0: line1

line3]
[: 1]
[ObjY (0x8504f2c): 2]
Filter: include []
Close
Filter: exclude [ObjX]
Close
[ObjY (0x8504f2c): 2]
[ObjZ (0x824b5e0): 3]
Filter: exclude []
Close
[custom: 0: line1

line3]
[: 1]
[ObjY (0x8504f2c): 2]
[ObjZ (0x81b4740): 3]
Filter: include all
Close
[custom: 0: line1

line3]
[: 1]
[ObjY (0x8504f2c): 2]
[ObjZ (0x81b4740): 3]
Filter: exclude all
Close"""

# ----------------------------------------------------

@UtTest
def CoreLoggerTest():
    
    RP = CoreLogRecordProcessor

    class LogStream(IBaseLogStream):
        def __init__(self, 
                     testOutputFn,
                     filterFn = RP.CreateIncludeAllFilterFn(), 
                     formatFn = RP.CreateFormatFn("[{textID}: {text}]", ""),
                     outputFormat = RP.OutputFormat.DontSplitLines, 
                     dropSize  = -1, printAdd = True):
            
            def OnDropFn():
                testOutputFn("Log was dropped")
            
            self.storage = CoreLogStore(OnDropFn, dropSize)
            self.formatFn = formatFn
            self.filterFn = filterFn
            self.ouputFn = RP.CreateOutputFn(testOutputFn, outputFormat)
            self.printAdd = printAdd
            self.testOutputFn = testOutputFn
        
        def __len__(self):
            return len(self.storage)
        
        def Add(self, record : CoreLogRecord):
            self.storage.Add(record)
            if self.printAdd: self.testOutputFn("Add: " + str(record))
                
        def Close(self):
            self.testOutputFn("Close")
            RP.Process(self.storage, self.ouputFn, self.filterFn, self.formatFn)
            self.storage.Clear()
    
    class TraceableObj:
        def __init__(self, log : CoreLogger):
            self.log = log
            
        def Trace(self, text : str, textID : str = None):
            self.log.Add(self, text, textID)
    
    class ObjX(TraceableObj): 
        def __init__(self, log : CoreLogger):
            TraceableObj.__init__(self, log)
            
    class ObjY(TraceableObj): 
        def __init__(self, log : CoreLogger):
            TraceableObj.__init__(self, log)
    
    class ObjZ(TraceableObj): 
        def __init__(self, log : CoreLogger):
            TraceableObj.__init__(self, log)

    
    def TestClose(outputFn):
        outputFn("TestClose")
        log = CoreLogger()
        st1 = LogStream(outputFn)
        st2 = LogStream(outputFn)
        log.AddOutput(st1)
        log.AddOutput(st2)
        assert log.OutputStreamCount == 2
        
        ObjX(log).Trace('0')
        assert len(st1) == 1
        assert len(st2) == 1
        
        log.Close() # print log, clear it, remove stream
        assert len(st1) == 0
        assert len(st2) == 0
        assert log.OutputStreamCount == 0
    
    def TestAddRemoveStreams(outputFn):
        outputFn("TestAddRemoveStreams")
        log = CoreLogger()
        st1 = LogStream(outputFn)
        st2 = LogStream(outputFn)
        log.AddOutput(st1)
        log.AddOutput(st2)
        fn = lambda: log.AddOutput(st2)
        AssertForException(fn, AssertionError)
        assert log.OutputStreamCount == 2
        
        log.RemoveOutput(st1)
        log.RemoveOutput(st2)
        fn = lambda: log.RemoveOutput(st1)
        AssertForException(fn, AssertionError)
        assert log.OutputStreamCount == 0
        log.Close()
    
    def TestFormat(outputFn):
        outputFn("TestFormat")
        
        def Impl(desc : str, recFormat : str, dtFormat : str, ):
            outputFn(desc)
            log = CoreLogger()
            log.AddOutput(LogStream(outputFn, 
                            formatFn = RP.CreateFormatFn(recFormat, dtFormat)))
            
            ObjX(log).Trace("0")
            log.Close() # print log, clear it, remove stream
        
        Impl('Format without dt', 
             '[{textID}: {text}]', '')
        Impl('Format with dt', 
             '<{date}> [{textID}: {text}]', "%y.%m.%d %H:%M:%S.%q")
        
    def TestFilter(outputFn):
        outputFn("TestFilter")
        
        def Check(desc : str, filterFn, outputFormat = RP.OutputFormat.DontSplitLines):
            outputFn(desc)
            log = CoreLogger()
            x = ObjX(log)
            y = ObjY(log)
            z = ObjZ(log)
            log.AddOutput(LogStream(outputFn, filterFn, 
                        RP.CreateFormatFn('[{textID}: {text}]'), 
                        outputFormat, -1, False))
            x.Trace('0: line1\n\nline3', 'custom')
            x.Trace('1', '')
            y.Trace('2', None)
            z.Trace('3')
            log.Close()
        
        Format = RP.OutputFormat
        Check("Filter: include [ObjX, ObjY], split lines, skip empty",
              RP.CreateIncludeFilterFn([ObjX, ObjY]),
              Format.SplitLinesSkipEmpty)
        
        Check("Filter: include [ObjX, ObjY], split lines, don't skip empty",
              RP.CreateIncludeFilterFn([ObjX, ObjY]), Format.SplitLines)
        
        Check("Filter: include [ObjX, ObjY], don't split lines",
              RP.CreateIncludeFilterFn([ObjX, ObjY]), Format.DontSplitLines)
        
        Check("Filter: include []", 
              RP.CreateIncludeFilterFn([]), Format.SplitLines)
        
        Check("Filter: exclude [ObjX]", 
              RP.CreateExcludeFilterFn([ObjX]))
        
        Check("Filter: exclude []", RP.CreateExcludeFilterFn([]))
        
        Check("Filter: include all", RP.CreateIncludeAllFilterFn())
        
        Check("Filter: exclude all", RP.CreateExcludeAllFilterFn())
    
    
    def CheckResult(newResult : str, correctResult : str, outputFn):
        # ignore date and object id
        outputFn ("CheckResult")
        
        def SkipDateAndId(line : str) -> str:
            # line: <date> [ text (id): text]
            CDateStart = '<'
            CDateEnd = '>'
            CId = '0x'
            res = line
            if line.startswith(CDateStart):
                res = res[(res.find(CDateEnd) + 1):]
            idStart = res.find(CId)
            if idStart >= 0:
                
                def IsHexChar(ch) -> bool:
                    ch = ch.lower()
                    return ch.isdigit() or ch in ['a', 'b', 'c', 'd', 'e', 'f']
            
                idEnd = idStart + len(CId)
                while IsHexChar(res[idEnd]):
                    idEnd += 1
                res = res[:idStart] + res[idEnd:]
            return res
        
        # test comments, displayed via pr() ignored in test
        newLines = newResult.splitlines()
        correctLines = correctResult.splitlines()
        
        if len(newLines) != len(correctLines):
            print("New file lines:\n", newLines)
            print("Correct file lines:\n", correctLines)
            assert len(newLines) == len(correctLines)
            
        for i, line in enumerate(correctLines):
            correctLine = SkipDateAndId(line)
            newLine = SkipDateAndId(newLines[i])
            outputFn ("[{0}]\n[{1}]\n\n".format(newLine, correctLine))
            assert newLine == correctLine
    
    def Impl(outputFn):
        results = []

        def pr(text : str):
            outputFn(text)
            results.append(text)
        
        TestClose(pr)
        TestAddRemoveStreams(pr)
        TestFormat(pr)
        TestFilter(pr)

        CheckResult("\n".join(results), _CTestResult, pr)

        pr("CoreLoggerTest: OK")
        
    GTestRunner.Run(Impl, traceTest = False)
      
# ---------------------------------------------------- 

if __name__ == "__main__":
    
    import unittest
    unittest.main()
   
    
    