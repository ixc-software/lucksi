'''
Created on Jun 11, 2010

@author: root
'''


from DRIDomain.TelnetClientDriAsync import TelnetClientDriAsync, CommandDriHandler
from CDUtilsPack.MiscUtils import AssertForException



# --------------------------------------------------------------------

class TelnetClientDriAsyncQueue:
    """
    Wrapper for TelnetCleintDriAsync. Using to be able abort all DRI
    commands from object.
    """
    
    def __init__(self, dri : TelnetClientDriAsync, errorHandler):
        """
        'errorHandler' - fn(e : Exception) -> None
        """
        self.queue = [] # [CommandDriHandler]
        self.aborted = False
        
        def Process(cmdList : str or [str], callback) -> CommandDriHandler:
            
            def Completed(res : CommandDriHandler):
            
                def Remove():
                    for i in self.queue:
                        if i == res: 
                            self.queue.remove(i)
                            return
                    assert 0 and "Handler not found"
                
                assert res
                Remove()
                
#                if not res.ResultsOK:
#                    errorHandler(DriError(res.LastResult, res.CommandsStr))
#                    return
                
                callback(res)
            
            self.__CheckNotAborted()
            h = dri.Process(cmdList, Completed)
            assert h
            self.queue.append(h)
            return h
        
        self.processFn = Process
    
    
    def __len__(self):
        return len(self.queue)
    
    
    def __CheckNotAborted(self):
        assert not self.aborted
        
    
    def Process(self, cmds : str or [str], callback):
        """
        'cmd'      - DRI command or list of commands,
        'callback' - fn(res : CommandDriHandler) -> None
        """
        return self.processFn(cmds, callback)    
    
    
    def AbortAll(self):
        """
        Just abort all commands in inner list. Don't close TelnetClientDriAsync.
        """
        self.__CheckNotAborted()
        for item in self.queue:
            item.Abort()            
        self.aborted = True
        

# --------------------------------------------------------------------
# Tests
# --------------------------------------------------------------------

from TestDecor import UtTest
from AllTest.TestParams import TestParams

# --------------------------------------------------------------------

@UtTest
def TelnetClientDriAsyncQueueTest():
    
    CTraceTest = False
    
    class Test: 
        
        def __init__(self, p : TestParams):
            self.p = p
            self.q = TelnetClientDriAsyncQueue(self.p.DRI, 
                                               lambda e: p.ErrHandler(e))
            self.okCount = 0
            self.failCount = 0
            
            p.Output("Test TelnetClientDriAsyncQueue started...")              
            
            callback = lambda res : self.__QueueCmdCallback(res)
            
            h = self.q.Process("CS_Print 1", callback)
            assert h
            self.q.Process("CS_Print 2", callback)
            self.q.Process(["CS_Print 3", "CS_Print 4"], callback)
        
        
        def __PrintRes(self, text : str, res : CommandDriHandler):
            self.p.Output("{0}: {1}".format(text, res))
        
        
        def __QueueAbortedCmdCallback(self, res : CommandDriHandler):
            self.__PrintRes("QueueAbortedCmdCallback", res)
            assert res.IsAborted
            self.failCount += 1
            if self.failCount < 2: return
            
            assert len(self.q) == 0
            assert self.okCount == 3
            
            self.q = None
            
            self.p.Output("Test TelnetClientDriAsyncQueue: OK")
            self.p.Complete()
            
#            def __LastCallback(self, res : CommandDriHandler):
#                PrintRes("LastCallback", res)
#                assert len(self.q) == 0
#                
#                self.q = None
#                self.dri = None
#                
#                assert self.okCount == 3
#                assert self.failCount == 2
#                                                        
#                Output("Test TelnetClientDriAsyncQueue: OK")
#                completeFn()
        
        
        def __QueueCmdCallback(self, res : CommandDriHandler):
            
            def DriCmdCallback(res : CommandDriHandler):
                self.__PrintRes("DriCmdCallback", res)
                assert not res.IsAborted
                assert res.IsCompleted
        
            
            self.__PrintRes("QueueCmdCallback", res)
            self.okCount += 1
            assert res.IsCompleted
            assert not res.IsAborted
            assert res.ResultsOK
            
            if self.okCount < 3: return
            assert len(self.q) == 0
            
            self.p.dri.Process("_non_queue_cmd_", DriCmdCallback)
            callback = lambda res: self.__QueueAbortedCmdCallback(res)
            self.q.Process("_invalid_1_", callback)
            self.q.Process("_invalid_2_", callback)
            self.q.AbortAll() # abort commands in queue only
            
            #self.q.ProcessCmd("_last_", LastCallback)
            
            # can't process commands after AbortAll()
            def Check():
                self.q.Process("CS_Set", lambda res: self.__QueueCmdCallback(res))
            AssertForException(Check, AssertionError)

                  
    def Impl(p : TestParams):
        t = Test(p)
        
    from AllTest.TestRunner import GTestRunner
    GTestRunner.RunIpsius(Impl, traceTest = CTraceTest, startEventLoop = True,
                          withDri = True, enableDriTrace = CTraceTest)


# --------------------------------------------------------------------

if __name__ == "__main__":
    
    import unittest
    unittest.main()
    

