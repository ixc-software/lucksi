'''
Created on Jul 16, 2010

@author: root
'''
from PackUtils.CoreTrace import CoreTrace
from CDUtilsPack.Telnet.TelnetParams import TelnetParams
from CDUtilsPack.MiscUtils import AssertForException

class TestParams:
    """
    Common test params class.
    """
    def __init__(self, completeFn, outputFn,  
                 telnetParams : TelnetParams = None, domainName : str = None, 
                 createDri = False):
        """
        'completeFn'   - fn(exitCode : int) -> None, 
        'outputFn'     - fn(text : str) -> None,
        'traceCode'    - using inside CreateCoreTrace(),
        'telnetParams', 
        'domainName', 
        'createDri'    - valid only for TestRunners with ProjIpsius.
        """
        self.complete = False
        self.completeFn = completeFn
        self.outputFn = outputFn
        self.telnetParams = telnetParams
        self.domainName = domainName
        self.dri = None
        if createDri: self.__InitDri()
            
    
    def __del__(self):
        assert self.complete
        
    
    def __CheckNotComplete(self):
        assert not self.IsComplete()
    
    def __InitDri(self):
        self.__CheckNotComplete()
        from DRIDomain.TelnetClientDriAsync import TelnetClientDriAsync 
        self.dri = TelnetClientDriAsync(self.IpsiusDomainName, 
                       self.IpsiusTelnetParams, lambda e: self.ErrHandler(e),
                       self.CreateCoreTrace())
        
    
    def Complete(self, exitCode : int = 0, closeApp = True):
        """
        'closeApp' - using False in tests for GUI widgets to don't 
                     close widget at the and of test.
        """
        self.__CheckNotComplete()
        self.Output("Test exit with code: {}".format(exitCode))
        if exitCode != 0: self.Output("Failed!")
        if self.dri: 
            if not self.dri.IsClosed(): self.dri.Close()
            self.dri = None
        self.complete = True
        if closeApp: 
            self.completeFn(exitCode)
    
    
    def IsComplete(self) -> bool:
        return self.complete
     
        
    def Output(self, text : str):
        # self.__CheckNotComplete()
        self.outputFn(text)
    
    
    def ErrHandler(self, e : Exception):
        """
        Using to process Exceptions in async test to close test correctly.
        """
        raise e
       
       
    def CreateCoreTrace(self, enableTrace : bool = False) -> CoreTrace:
        self.__CheckNotComplete()
        Trace = lambda obj, text, textID: self.Output(text)
        return CoreTrace(Trace, enableTrace)
    
    
    def __CheckPropertyForIpsius(self, property, desc : str):
        self.__CheckNotComplete()
        if self.telnetParams != None: return
        raise AssertionError("No {} set! Use TestRunner with ProjIpsius.".
                             format(desc))
    
    def SetupDri(self, enableTrace : bool):
        assert self.dri
        #if not self.dri: self.__InitDri()
             
        from DRIDomain.TelnetClientDri import TelnetTraceStates
        self.dri.EnableTrace(TelnetTraceStates(enableTrace))
        
    
    # Those properties are valid only for TestRunners with ProjIpsius 
    
    @property
    def IpsiusTelnetParams(self) -> TelnetParams:
        self.__CheckPropertyForIpsius(self.telnetParams, "TelnetParams")
        return self.telnetParams
    
    
    @property
    def IpsiusDomainName(self) -> str:
        self.__CheckPropertyForIpsius(self.domainName, "domain name") 
        return self.domainName
    
    
    @property
    def DRI(self): # -> TelnetClientDriAsync
        self.__CheckNotComplete()
        assert self.dri 
        return self.dri
    

# ---------------------------------------------------------
# Test
# ---------------------------------------------------------

from TestDecor import UtTest

@UtTest
def TestParamsTest():
    
    CTraceTest = False
    
    class TestErr(Exception): pass
        
    def Complete(code : int): pass
    
    telnetParams = TelnetParams("127.0.0.2", 1589, "1", "x") 
    domainName = "domain"
    
    def WithoutDri(outputFn):
        
        def CompleteErr(code : int):
            if code != 0: raise TestErr()
    
        outputFn("TestParams without DRI")
        p = TestParams(CompleteErr, outputFn, telnetParams, domainName, False)
        
        p.Output("Check error handler")
        AssertForException(lambda: p.ErrHandler(TestErr()), TestErr)
        
        ct = p.CreateCoreTrace(CTraceTest)
        assert isinstance(ct, CoreTrace)
        
        assert p.IpsiusTelnetParams == telnetParams
        assert p.IpsiusDomainName == domainName
        
        p.Output("Check access to DRI")
        AssertForException(lambda : p.DRI, AssertionError)
        
        p.Output("Check that test failed if complete with err")
        AssertForException(lambda: p.Complete(1), TestErr)
    
        outputFn("TestParamsTest: OK")
    
    from AllTest.TestRunner import GTestRunner
    GTestRunner.Run(WithoutDri, CTraceTest)

# ---------------------------------------------------------

if __name__ == "__main__":
    
    import unittest
    unittest.main()
    
    
