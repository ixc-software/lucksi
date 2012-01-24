'''
Created on Jul 13, 2010

@author: root
'''



from TestDecor import UtTest
from PyQt4 import QtCore

from DRIDomain.CoreIpsiusRunnerBase import (CoreIpsiusCmdLineParams,
                                       CoreIpsiusRunnerBase)
from PackUtils.CoreCmdLineParams import CoreCmdLineParams
from CDUtilsPack.Telnet.TelnetParams import TelnetParams


from AllTest.TestParams import TestParams
import traceback
from PackUtils.CoreTrace import CoreTrace


# ------------------------------------------------------

class TestIpsiusRunner:
    """Using to run ProjIpsius for tests."""
    
    def __init__(self, onStartFn, outputFn, completeFn, 
                 withDri : bool, enableDriTrace : bool, ipsiusTrace : CoreTrace,
                 ipsiusExePath : str, udplogExePath : str):
        """
        'onStartFn' - test function: fn(closeFn, outputFn, port : int) -> None, 
        'completeFn' - function called when test finished: fn(code : int) -> None, 
        'outputFn' - test output function: fn(text : str) -> None, 
        """
        self.completeFn = completeFn
        self.impl = None
        cmdLine = ["self", "/e", ipsiusExePath, "/l", udplogExePath]
        sett = CoreCmdLineParams(cmdLine)
        params = CoreIpsiusCmdLineParams(TelnetParams("127.0.0.1", 0, "1", "test"), 
                                         sett, True)
        
        
        def Started(port : int):
            outputFn("Running: " + onStartFn.__name__)
            
            try:
                p = TestParams(lambda exitCode: self.Close(exitCode), 
                               outputFn, self.impl.params.TelnetParams, 
                               self.impl.params.DomainName, withDri)
                if withDri: p.SetupDri(enableDriTrace)
                onStartFn(p)
            except (Exception, AssertionError):
                traceback.print_exc() 
                self.Close(1)                
            
        def Finished(exitCode : int, exitStatus : QtCore.QProcess.ExitStatus):
            pass
            
        def Error(data : str):
            if data and data != "\n": 
                print (data)
                self.Close(1)
            
        self.impl = CoreIpsiusRunnerBase(Started, Finished, Error, outputFn,
                                         params, ipsiusTrace)
        self.impl.Run()
    
    
    def Close(self, exitCode = 0):
        self.impl.Close()
        self.completeFn(exitCode) # close app
            
               
#    @property
#    def Params(self) -> CoreIpsiusCmdLineParams:
#        return self.impl.Params

