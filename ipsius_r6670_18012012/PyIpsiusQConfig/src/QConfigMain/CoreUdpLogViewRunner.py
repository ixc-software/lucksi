'''
Created on Jul 8, 2010

@author: root
'''
from PyQt4 import QtCore
from PackUtils.CoreTrace import CoreTrace
from PackUtils.Misc import ToStr, QProcessErrorToStr
from PackUtils.CoreCmdLineParams import CoreCmdLineParams
from QConfigMain.CoreFieldValidator import ValidatorInt, ValidatorPort, ValidatorErr
from CDUtilsPack.MiscUtils import AssertForException
from PackUtils.CoreProcessRunner import CoreProcessRunner


class CoreUdpLogViewRunner:
    """
    Using to launch UdpLogviewer as separate process to monitor ProjIpsius 
    activity.
    """

    def __init__(self, onStartedFn, onFinishedFn, onErrorFn , onOutputFn, 
                 sett : CoreCmdLineParams, trace : CoreTrace):
        """
        See CoreProcessRunner description.
        """
        assert sett.RunUdpLogView
        self.sett = sett
        self.trace = trace
        self.udpLog = CoreProcessRunner(onStartedFn, onFinishedFn, onErrorFn, 
                                        onOutputFn, trace)
        self.ports = [] # ["port:type"]
        self.dbSize = 10000
        self.eachIntervalMs = None
        self.eachRecordCount = None 
        
        
    def DBSize(self, size : int): # raise ValidatorErr
        """
        UdpLogViewer database size.
        """
        v = ValidatorInt(1, allowNone = False)
        v.Check(size, "CoreUdpLogViewRunner.dbSize")
        self.dbSize = size
    
    
    def AddPort(self, port : int, dataType : str = "str"): # raise ValidatorErr
        """
        'port'     - ProjIpsius UDP log port, must be > 0,
        'dataType' - value from list: [str, wstr].
        """
        v = ValidatorPort()
        v.Check(port, "CoreUdpLogViewRunner.ports")
        if not dataType in ["str", "wstr"]:
            raise ValidatorErr(("unknown port data type (must be "
                                "'str' or 'wstr')"), dataType, 
                                "CoreUdpLogViewRunner.ports")
        self.ports.append("{0}:{1}".format(port, dataType))
    
    
    def RecordingToFile(self, eachRecordCount : int, 
                        eachIntervalMs : int): # raise ValidatorErr
        """
        'eachRecordCount' - if not None, recording to file each N records,
        'eachIntervalMs'  - if not None, recording to file each N milliseconds.
        """
        v = ValidatorInt(1, allowNone = True)
        v.Check(eachRecordCount, "CoreUdpLogViewRunner.eachRecordCount")
        v.Check(eachIntervalMs, "CoreUdpLogViewRunner.eachIntervalMs")
        self.eachRecordCount = eachRecordCount
        self.eachIntervalMs = eachIntervalMs
        
    
    def Run(self):
        
        def MakeArgs():
            assert len(self.ports) > 0
            res = ["-pt"]
            res += self.ports
            res += ["-s", str(self.dbSize)]
            if not self.eachIntervalMs and not self.eachRecordCount:
                res.append("-norec")
            if self.eachRecordCount:
                res += ["-rec_count", self.eachRecordCount]
            if self.eachIntervalMs:
                res += ["-rec_int", self.eachIntervalMs] 
            return res           
        
        self.udpLog.Run(self.sett.UdpLogViewFile, MakeArgs())     
    
    
    def Close(self):
        self.udpLog.Close()        
            

# ---------------------------------------------------------
# Test
# ---------------------------------------------------------

from TestDecor import UtTest
from AllTest.TestRunner import GTestRunner
from AllTest.TestParams import TestParams


@UtTest
def CoreUdpLogViewRunnerTest():
    
    class Test:
        def __init__(self, p : TestParams):
            sett = CoreCmdLineParams(["file", "/e", GTestRunner.IpsiusPath,
                                      "/l", GTestRunner.UdpLogPath])
            Output = lambda data: None
            
            def Finished(exitCode, exitStatus): 
                if not p.IsComplete: p.Complete(exitCode)                
                            
            def Error(data : str):
                p.Output("Error: " + data)
                if "QtCore.QProcess.Crashed" in data:
                    p.Complete(0)
                else : p.Complete(1)                    
            
            def Started():
                p.Output("UdpLogViewer is started.")
                self.l.Close()
            
            self.l = CoreUdpLogViewRunner(\
                            Started, Finished, Error, Output,
                            sett, p.CreateCoreTrace())
            
            self.__CheckAddPort()
            self.__CheckSetRecording()
            self.__CheckSetDBSize()                      
            self.l.Run()
                        
        def __CheckAddPort(self):
            fn = lambda: self.l.AddPort(-1, "str")
            AssertForException(fn, ValidatorErr)
            fn = lambda: self.l.AddPort(2000, "xstr")
            AssertForException(fn, ValidatorErr)
            self.l.AddPort(65001, "str")
                        
        def __CheckSetRecording(self):
            self.l.RecordingToFile(2, None)
            self.l.RecordingToFile(None, 10)
            self.l.RecordingToFile(None, None)
            
        def __CheckSetDBSize(self):
            fn = lambda: self.l.DBSize(-1)
            AssertForException(fn, ValidatorErr)
            fn = lambda: self.l.DBSize(0)
            AssertForException(fn, ValidatorErr)
            fn = lambda: self.l.DBSize(None)
            AssertForException(fn, ValidatorErr)
            self.l.DBSize(1000)
           
    
    def Impl(p : TestParams):
        t = Test(p)        
    
    GTestRunner.RunApp(Impl, traceTest = False)

# ---------------------------------------------------------

if __name__ == "__main__":
    
    import unittest
    unittest.main()
    


        