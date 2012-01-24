'''
Created on Jul 12, 2010

@author: root
'''

from PyQt4 import QtCore
from PackUtils.CorePathes import TempFiles
from DRIDomain.CoreIpsiusOutputParser import CoreIpsiusOutputParser
from PackUtils.CoreTrace import CoreTrace
from CDUtilsPack.Telnet.TelnetParams import TelnetParams
from PackUtils.Misc import ToStr, QProcessErrorToStr
from PackUtils.CoreCmdLineParams import CoreCmdLineParams
from CDUtilsPack.MetaUtils import Property
from PackUtils.CoreProcessRunner import CoreProcessRunner
import os

# -----------------------------------------------------

#class ICoreIpsiusRunnerEvents:
#    """
#    Base interface for CoreProcessRunner owner.
#    """
#    def Started(self, port : int): pass
#    def Finished(self, exitCode : int, 
#                 exitStatus : QtCore.QProcess.ExitStatus): pass
#    def Error(self, data : str): pass
#    def Output(self, data : str): pass
    
# -----------------------------------------------------

class CoreIpsiusCmdLineParams:
    """
    List of command line params to run ProjIpsius.
    """
    
    def __init__(self, telnetParams : TelnetParams, sett : CoreCmdLineParams,
                 useAutorunScript = True): 
        self.domainName = "Domain"
        self.telnetParams = telnetParams
        self.scriptDir = TempFiles.DirScript 
        self.udpLogPort = 65001
        self.sett = sett
        
        def GenDefaultAutorunScript() -> str: # return name
            script = """
                ObjCreate NObjHwFinder HwFinder true
                .LogLevel = 3                
                .FirmwarePath = "{0}"
                .Enabled = true
                ObjCreate  NObjResipLog "ResipLog"
                ExitOnLastSessionDrop = true
                TelnetServer.LogLevel = 3
                Log.Timestamp = true
                """.format(self.sett.FirmwareDir)
            file = TempFiles.ScriptFile("gen_IpsiusAutorun.isc")
            with open(file, 'w') as f:
                f.write(script)            
            return file
        
        self.autorunScriptName = GenDefaultAutorunScript()
    
    
    def Generate(self) -> []:
        """
        Return list of ProjIpsius command line flags and their value in format:
        [flag1, value1, flag2, flag3, value31, value32, flag4] 
        """ 
        host = "{0}:{1}".format(self.telnetParams.Host, self.telnetParams.Port)
        res = ["-v", 
               "-n", self.domainName, 
               "-t", host, 
               "-p", self.telnetParams.Password,
               "-i", self.scriptDir,
               "-sc", self.autorunScriptName,
               "-l", str(self.udpLogPort),
               "-ne"]
        return res


    DomainName = Property("domainName", True)
    TelnetParams = Property("telnetParams", True)
    UdpLogPort = Property("udpLogPort", True)
    ScriptFile = Property("autorunScriptName")
    
    
    @property
    def ExeFile(self) -> str:
        return self.sett.ExeFile
    
# -----------------------------------------------------

class _RunnerState:
    Init = 0
    Started = 1
    WasError = 2
    WasClosed = 3 
    Finished = 4
     

# -----------------------------------------------------

class CoreIpsiusRunnerBase:
    """
    Using to run ProjIpsius as separate process. 
    """
  
    def __init__(self, onStartedFn, onFinishedFn, onErrorFn , onOutputFn, 
                 params : CoreIpsiusCmdLineParams,
                 trace : CoreTrace):
        """
        'onStartFn'  - fn(port : int) -> None
        'onFinishFn' - fn(exitCode : int, exitStatus : QtCore.QProcess.ExitStatus)
        'onErrorFn'  - fn(data : str)
        'onOutputFn' - fn(data : str)
        """
        self.onErrorFn = onErrorFn
        
        def Started():
            pass 
                
        def Finished(exitCode : int, 
                     exitStatus : QtCore.QProcess.ExitStatus):
            assert not self.IsFinished
            self.state = _RunnerState.Finished
            sc = params.ScriptFile
            if os.path.exists(sc): os.remove(sc)
            onFinishedFn(exitCode, exitStatus)
    
        
        Output = lambda data: self.__Output(data, onOutputFn, onStartedFn, params)
        Error = lambda data: self.__Error(data) 
        
        self.params = params
        self.state = _RunnerState.Init
        self.startupParser = CoreIpsiusOutputParser()
        #self.closeViaTelnet = False
        self.process = CoreProcessRunner(Started, Finished, Error, Output, trace)
    
    
    def __Error(self, data : str): 
            # ignore error after closed
            if self.state == _RunnerState.WasClosed: return
            self.state = _RunnerState.WasError
            self.onErrorFn(data)
            
    
    def __Output(self, data : str, onOutputFn, onStartedFn, 
                 params : CoreIpsiusCmdLineParams):
        if self.state != _RunnerState.Init:
            onOutputFn(data)
            return
        
        self.startupParser.AddData(data)
        # aborted
        if self.startupParser.WasError and self.startupParser.Completed:
            self.__Error(self.startupParser.IpsiusOutput)
            return
        # get port 
        TelnetParams.Port = self.startupParser.IpsiusPort
        # check script completed
        if not self.startupParser.ScriptCompleted: return
        if self.startupParser.WasError:
            self.__Error(self.startupParser.IpsiusOutput)
            return
        #self.closeViaTelnet = True
        onOutputFn(self.startupParser.IpsiusOutput)
        self.state = _RunnerState.Started
        onStartedFn(params.TelnetParams.Port)
        
        
    def Run(self):
        assert not self.IsStarted
        self.process.Run(self.params.ExeFile, self.params.Generate()) 
    
    
    def Close(self):
        """
        Terminate process, cause it to crash. ProjIpsius close itself after 
        last session closed. Note: can avoid QProcess.Crashed error on Linux
        by closing ProjIpsius via DomainExit.
        """
        # assert self.IsStarted
        self.state = _RunnerState.WasClosed
        self.process.Close()      
    
    
    @property
    def IsClosed(self) -> bool:
        return self.state == _RunnerState.WasClosed
    
    
    @property
    def IsFinished(self) -> bool:
        return self.state == _RunnerState.Finished
        
    
    @property
    def IsStarted(self) -> bool:
        return self.state == _RunnerState.Started
    
    
    @property
    def WasError(self) -> bool:
        return self.state == _RunnerState.WasError
            
    Params = Property("params")
             
# --------------------------------------------------------
# Tests
# --------------------------------------------------------

from TestDecor import UtTest
from AllTest.TestRunner import GTestRunner
from AllTest.TestParams import TestParams


@UtTest
def CoreIpsiusRunnerBaseTest():
    
    CTraceTest = False
    
    def Impl(p : TestParams):
        
        class Test:
            def __init__(self):
                self.runner = None # CoreIpsiusRunnerBase
                self.output = ""
                                
                self.fwDir = TempFiles.TestDir("_test_fwdir")
                self.confDir = TempFiles.TestDir("_test_configdir")
                sett = CoreCmdLineParams(["file", 
                                          "/e", GTestRunner.IpsiusPath,
                                          "/c", self.confDir,
                                          "/f", self.fwDir])
                telnetParams = TelnetParams("127.0.0.1", 0, "1", "test")
                params = CoreIpsiusCmdLineParams(telnetParams, sett, 
                                                 useAutorunScript = True)
                
                def TestExit(code : int):
                    p.Output("TestExit")
                    def Remove(dir : str):
                        assert os.path.exists(dir)
                        os.removedirs(dir)
                    
                    assert not self.runner.IsClosed
                    
                    Remove(self.fwDir)
                    Remove(self.confDir)
                    # 0  - ok
                    # -2 - ok, esc pressed or DomainExit called
                    #if not self.runner.IsClosed: self.runner.Close()
                    p.Complete(code)
                
                def Started(port : int):
                    p.Output("Ipsius is started on port: {}".format(port))
                    assert self.output
                    assert self.runner.IsStarted
                    assert self.runner.Params.TelnetParams.Port == port
                    self.runner.Close()
                        
                def Finished(exitCode : int, 
                             exitStatus : QtCore.QProcess.ExitStatus):
                    p.Output("Ipsius finished: code = {0}, status = {1}".\
                       format(exitCode, exitStatus))
                    if self.runner.WasError: return
                    
                    assert not self.runner.IsStarted
                    assert not os.path.exists(self.runner.Params.ScriptFile)
                    
                    if exitCode == 0 or exitStatus == QtCore.QProcess.Crashed:
                        TestExit(0) # ok
                    else:
                        TestExit(exitCode)
                        
                def Error(data : str):
                    p.Output("Ipsius error:" + data)
                    self.wasError = True
                    TestExit(1)               
                        
                def Output(data : str):
                    self.output += data
                    p.Output("Ipsius output: " + data)  
            
                
                self.runner = CoreIpsiusRunnerBase(\
                         Started, Finished, Error, Output,
                         params, p.CreateCoreTrace())
                assert self.runner.Params.UdpLogPort > 0
                self.runner.Run()   
            
                
                     
        t = Test()
                
    
    GTestRunner.RunApp(Impl, CTraceTest, startEventLoop = True)
    
# --------------------------------------------------------

if __name__ == "__main__":
    import unittest
    unittest.main()
    
    