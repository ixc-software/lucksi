'''
Created on Jun 10, 2010

@author: root
'''

from PyQt4 import QtCore
import threading
from CDUtils import Property

from PackUtils.Misc import CheckFnArgs
from PackUtils.CoreTrace import CoreTrace
from CDUtilsPack.Telnet.TelnetClient import TelnetParams, TelnetError, TelnetTimeout
from DRIDomain.TelnetClientDri import (CommandDriResult, TelnetClientDri, DriError,
                                  TelnetTraceStates, CDefOneCommandTimeoutSec)
from CDUtilsPack.Telnet.TelnetSocket import SocketError
from queue import Queue
from CDUtilsPack.MiscUtils import AssertForException
from types import FunctionType



# --------------------------------------------------------------------

class _CommandDriHandlerImpl: pass # forward declaration

# --------------------------------------------------------------------

class CommandDriHandler():
    """
    Handler to DRI command(s) in queue. Returned by 
    TelnetClientDriAsync.Process() and allow user to abort added 
    command. Returned as parameter of callback to user when command(s) was 
    processed.
    """
    
    def __init__(self, impl : _CommandDriHandlerImpl):
        self.impl = impl
        
    def __str__(self):
        
        def Item(name : str, val : bool) -> str:
            return "{0}: {1}\n".format(name, val)
        
        res = "CommandDriHandler:\n"
        res += Item("IsStarted", self.IsStarted)
        res += Item("IsCompleted", self.IsCompleted)
        res += Item("IsAborted", self.IsAborted)
        res += Item("Commands", "; ".join(self.Commands)) 
        res += "Results:\n"
        for i in self.Results:
            res += "{}\n".format(i.Raw)
        res += Item("ResultsOK", self.ResultsOK)   
        return res    
        
    
    def __eq__(self, other) -> bool:
        return self.impl == other.impl
    
    
    def Abort(self):
        """
        Mark command aborted. If command processing wasn't started yet don't 
        execute it and send callback to user.        
        """
        self.impl.Abort()
        
    
    @property
    def IsStarted(self): return self.impl.IsStarted
    
    
    @property
    def IsCompleted(self): return self.impl.IsCompleted
    
    
    @property
    def IsAborted(self): return self.impl.IsAborted
    
    
    @property
    def Results(self): return self.impl.Results
    
    
    @property
    def Commands(self): return self.impl.Commands
    
    
    @property
    def CommandsStr(self): return "; ".join(self.impl.Commands) 
    
    
    @property
    def LastResult(self) -> CommandDriResult:
        """Check that list of results is not empty and return last one."""
        size = len(self.impl.Results)
        # if size <= 0: return
        assert size > 0        
        return self.impl.Results[size - 1]
    
    
    @property
    def ResultsOK(self):
        """
        Return True if list of results is empty or last result is OK (what
        means that all results in list is OK also). Otherwise, return False.
        """
        if len(self.Results) <= 0: return True
        return self.LastResult.OK
        

# --------------------------------------------------------------------

class _CommandDriHandlerImpl:
        """Handler to DRI command or list of DRI commands in queue."""
        
        def __init__(self, dri : TelnetClientDri, 
                     cmdList : str or [str], runCallback):
            """
            'cmdList'      - list of DRI commands,
            'runCallback'  - fn(result : _CommandDriHandlerImpl) -> None, called when all
                             commands in 'cmdList' was completed or aborted.
            """
            CheckFnArgs(runCallback, (_CommandDriHandlerImpl,), None) 
            # assert len(cmdList) > 0
            
            self.isStarted = False
            self.isCompleted = False
            self.isAborted = False
            self.cmdList = [cmdList] if isinstance(cmdList, str) else cmdList
            self.results = [] # CommandDriResult
            
            def Run():
                if self.isAborted:
                    runCallback(self) # no processing, just send callback 
                    return
                    
                assert (not self.isCompleted and not self.isStarted)
                self.isStarted = True
                self.results = dri.Process(cmdList)
                self.isStarted = False
                self.isCompleted = True
                runCallback(self)
            
            self.runFn = Run
        
        
        def Process(self):
            """
            Process command using TelnetClientDri. Command can't be aborted 
            while processed. 
            """
            self.runFn()
            
        
        def Abort(self):
            """
            Mark command aborted. If command isn't started and isn't completed 
            send callback to user.
            """
            assert not self.isAborted
            # if self.isAborted: return
            
            self.isAborted = True
            
        
        def CreateUserHandler(self) -> CommandDriHandler:
            """
            Make read-only wrapper of command handler for command's sender.
            """
            return CommandDriHandler(self)
        
        
        IsStarted    = Property('isStarted')
        IsCompleted  = Property('isCompleted')
        IsAborted    = Property('isAborted')
        Results      = Property('results')
        Commands     = Property('cmdList')


# --------------------------------------------------------------------


class TelnetClientDriAsync(QtCore.QThread):
    """
    Using execute DRI commands in the separate thread. Execute result callbacks 
    in the main thread. callback -- fn(CommandDriResult) -> None. Catch all 
    DRI, Telnet and Socket errors in Telnet thread and send them via 
    ErrorHandler(e : Exception) -> None above.
    """   
    
    
    
    def __init__(self, domainName : str, params : TelnetParams, 
                 errorHandler, trace : CoreTrace, connectionHandler = None, autoPolling = False):
        """
        'domainName'   - ProjIpsius domain name,
        'params'       - parameters using to connect to the ProjIpsius, 
        'errorHandler' - fn(e : Exception) -> None.
        'connectionHandler' - fn(params : TelnetParams, conected : bool), if None return only after connected
        'autoPolling' - auto polling connect state by sending empty dri command.
        """
        QtCore.QThread.__init__(self)
        
        assert isinstance(domainName, str) or domainName == None 
        assert isinstance(params, TelnetParams)
        assert isinstance(trace, CoreTrace)
        CheckFnArgs(errorHandler, (Exception,), None), 
        
        self.trace = trace
        self.closed = False
        self.queue = Queue() # [callable object]
        
        self.dri = None        
        
        self.__ResultSig.connect(lambda cllbackFn: cllbackFn())
        self.__ConnectSig.connect(lambda ok : connectionHandler(params, ok)) 
        self.__TelnetTreadErrSig.connect(errorHandler)        

        def Connect(): # can raise Exception            
            self.dri = None
            self.dri = TelnetClientDri(domainName, params, trace.Copy()) # can raise Exception            
        self.connectFn = Connect                                                       
        
        lock = threading.Lock()
        condConnect = threading.Condition(lock)                        
        
        def ConnectedStateNotify(ok : bool):
            '''Called from dri thread'''
            
            if connectionHandler != None:
                self.__ConnectSig.emit(ok)
                return
            
            # unlock constructor
            with lock:
                if ok: condConnect.notify()     
                
        
        self.ConnectedStateNotify = ConnectedStateNotify
        
        # autoPolling
        def SendNone():            
            if self.dri == None: return            
            self.Process(' ', lambda res :None)
        
        if autoPolling:            
            self.poll = QtCore.QTimer()
            self.poll.timeout.connect(SendNone)
            self.poll.start(5000)  
        
        # start thread:
        with lock:
            self.start(QtCore.QThread.LowPriority)  
            if connectionHandler == None: condConnect.wait()    
    
    
    def __del__(self):
        assert self.closed       
        self.wait()
        
    
    def __CheckNotClosed(self):
        assert not self.IsClosed()
    
    
    __ResultSig = QtCore.pyqtSignal(FunctionType)#QtCore.pyqtWrapperType)
    __ConnectSig = QtCore.pyqtSignal(FunctionType)
    __TelnetTreadErrSig = QtCore.pyqtSignal(QtCore.pyqtWrapperType)
        
    
    def run(self): # override             
                
        connected  = False
                    
        # Telnet thread loop
        while True:
            # connect loop
            #try connect if not connected
            while not connected:
                try:                                   
                    self.connectFn()                                        
                    connected = True
                    self.ConnectedStateNotify(connected)                                      
                except SocketError: pass                                                                          
            
            try:
                while True: 
                    if self.closed and self.queue.qsize() == 0:
                        break
                    fn = self.queue.get()
                    fn()        
                
            # disconected
            except (DriError, TelnetError, TelnetTimeout, SocketError) as e: 
                connected = False
                self.ConnectedStateNotify(connected)                 
                self.__TelnetTreadErrSig.emit(e) # keep old logic
                
               
    
    def __Trace(self, text : str):
        self.trace.Add(self, text)   
    
    
    def __Add(self, callableItem):
        self.queue.put(callableItem)
        
    
    def Process(self, cmdList : str or [str], callback) -> CommandDriHandler:
        """
        'cmdList' - DRI command or list of commands,
        'callback' - fn(res : CommandDriHandler) -> None,
        'timeoutSec' - executing all commands in list timeout.
        """
        assert callback
        
        def Add() -> CommandDriHandler:
            
            def RunCallback(item : _CommandDriHandlerImpl):    
                
                def SyncCallback():
                    # Inside main thread
                    res = item.CreateUserHandler()
                    #if not res.ResultsOK: # and not res.IsAborted:
                    #    err = DriError(res.LastResult, res.CommandsStr)
                    #    self.__TelnetTreadErrSig.emit(err)
                    callback(res)
                
                # Inside Telnet thread
                self.__ResultSig.emit(SyncCallback)
            
            item = _CommandDriHandlerImpl(self.dri, cmdList, RunCallback)
            self.__Add(item.Process)
            res = item.CreateUserHandler()
            assert res
            return res 
        
        # Inside main thread
        self.__CheckNotClosed()
        self.__Trace("Process command(s): {}".format(cmdList))
        
        return Add()
    
    
    def Close(self):
        """
        Stop Telnet thread and wait for thread to exit process loop.
        """
        self.__CheckNotClosed()
        # set break flag
        self.closed = True
        # add item to queue to check break flag
        self.__Add(lambda: None)       
    
    
    def IsClosed(self) -> bool:
        return self.closed
    
    
    def EnableTrace(self, states : TelnetTraceStates):
        self.trace.Enable(states.ForDriAsync) 
        self.dri.EnableTrace(states)        

    
# --------------------------------------------------------------------
# Tests
# --------------------------------------------------------------------

from TestDecor import UtTest
from AllTest.TestParams import TestParams

# --------------------------------------------------------------------

@UtTest
def TelnetClientDriAsyncTest():
    
    CTraceTest = False
    
    class Test:
        
        def __init__(self, p : TestParams):
            self.p = p
            self.ta = TelnetClientDriAsync(p.IpsiusDomainName, 
                                           p.IpsiusTelnetParams, 
                                           lambda e: p.ErrHandler(e), 
                                           p.CreateCoreTrace(CTraceTest))
            assert not self.ta.IsClosed()
            self.ta.EnableTrace(TelnetTraceStates(True))
            self.ta.EnableTrace(TelnetTraceStates(False))
            self.okRecvCount = 0
            self.abortAllCount = 0
            self.failedRecv = False
            self.instAbortedRecv = False
            self.cmdHandler = None                
            
            self.p.Output("Test TelnetClientDriAsync started...")
            
            h = self.__Process("CS_Set", 
                               lambda res : self.__InstantlyAborted(res))
            assert not h.IsStarted
            assert not h.IsCompleted
            assert not h.IsAborted
            h.Abort()
            assert h.IsAborted
            h = None
                            
            self.__Process("ObjList", lambda res : self.__CompletedOk(res))            
            self.__Process("CS_Print", lambda res : self.__CompletedOk(res))
            self.__Process("_invalid_", lambda res : self.__CompletedFail(res))
            self.cmdHandler = \
                self.__Process("ObjList", 
                               lambda res : self.__AbortedInCallback(res))
        
            
        def __OutputRes(self, text : str, res : CommandDriHandler):
            self.p.Output("{0}:\n{1}".format(text, res))
        
        
        def __Process(self, cmd : str, callback) -> CommandDriHandler:
            return self.ta.Process(cmd, callback)
            
        
        def __InstantlyAborted(self, res : CommandDriHandler):
            self.__OutputRes("InstantlyAborted", res)
            assert not self.instAbortedRecv
            self.instAbortedRecv = True
            
            assert res.IsAborted
            assert not res.IsCompleted
            assert not res.IsStarted
            assert res.ResultsOK 
        
               
        def __CompletedOk(self, res : CommandDriHandler):
            self.__OutputRes("CompletedOk", res)
            assert res.ResultsOK
            assert res.IsCompleted
            assert not res.IsAborted
            assert not res.IsStarted
            
            self.okRecvCount += 1
        
        
        def __CompletedFail(self, res : CommandDriHandler):
            self.__OutputRes("CompletedFail", res)
            
            assert not self.failedRecv
            self.failedRecv = True
                                
            assert not res.ResultsOK
            assert not res.LastResult.OK
            assert res.IsCompleted
            assert not res.IsAborted
            assert not res.IsStarted
            
            # try abort self
            res.Abort()
            assert res.IsAborted
            self.__OutputRes("Complete Fail & aborted", res)
            
            # try abort other
            assert self.cmdHandler
            self.cmdHandler.Abort()
            assert self.cmdHandler.IsAborted                   
        
                            
    #                def __AbortAllCallback(self, res : CommandDriHandler):
    #                    self.abortAllCount += 1
    #                    OutputRes("AbortAllCallback({0})".format(self.abortAllCount),
    #                              res)
    #                    
    #                    if self.abortAllCount != 3: return
    #                    
    #                    assert self.okRecvCount == 2
    #                    assert self.failedRecv == True
    #                    assert self.instAbortedRecv == True
    #                    
    #                    # can't add any command after aborted
    #                    wasAssert = False
    #                    try:
    #                        Process("dummi", CompletedFail)
    #                    except AssertionError:
    #                        wasAssert = True
    #                    assert wasAssert and "No test assertion!"
    #                    
    #                    self.cmdHandler = None
    #                    self.ta = None
    #                    Output("Test TelnetClientDriAsync: OK")
    #                    completeFn()
    #                
    #                def __AbortAll(self, res : CommandDriHandler):
    #                    Process("CS_Set", AbortAllCallback)
    #                    Process("CS_Set", AbortAllCallback)
    #                    Process("CS_Set", AbortAllCallback)
    #                    self.ta.AbortAll()
    
        def __LastCallback(self, res : CommandDriHandler):
            assert self.okRecvCount == 2
            assert self.failedRecv == True
            assert self.instAbortedRecv == True
            self.ta.Close()
            assert self.ta.IsClosed()
            
            # can't add any command after closed
            
            def CheckAssert():
                self.__Process("dummi", lambda res: self.__CompletedFail(res))
                
            AssertForException(CheckAssert, AssertionError)
            
            self.cmdHandler = None
            self.ta = None
            self.p.Output("Test TelnetClientDriAsync: OK")
            self.p.Complete()
        
        
        def __AbortedInCallback(self, res : CommandDriHandler):
            assert self.cmdHandler == res
            
            # was aborted in __CompletedFail()
            self.__OutputRes("AbortedInCallback", res)
            # assert res.IsCompleted ?
            assert res.IsAborted
            
            self.__Process("CS_Print", lambda res: self.__LastCallback(res)) 
            
    
    def Impl(p : TestParams):
        t = Test(p)
        
    from AllTest.TestRunner import GTestRunner
    GTestRunner.RunIpsius(Impl, traceTest = CTraceTest, startEventLoop = True)
    
# --------------------------------------------------------------------

if __name__ == "__main__":
    
    import unittest
    unittest.main()
    
