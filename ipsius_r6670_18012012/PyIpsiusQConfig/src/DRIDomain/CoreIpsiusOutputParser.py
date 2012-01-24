'''
Created on Jun 16, 2010

@author: root
'''

from CDUtilsPack.MetaUtils import Property


class CoreIpsiusOutputParser:
    """
    Stream parser for ProjIpsius startup output.
    'data'            - all checked ipsius output,
    'scriptCompleted' - True if autorun script was completed,
    'wasError'        - True if ProjIpsius was aborted with exception or 
                        script was completed with error,
    'ipsiusPort'      - opened port for Telnet connections (not 0), 
    'completed'       - True if parser reach end of startup info.
    """
    def __init__(self):
        self.data = ""
        self.scriptCompleted = False
        #self.scriptCompletedWithErr = False
        self.wasError = False # aborted, exception or DRI error in script
        self.ipsiusPort = None
        self.completed = False
    
    
    def __ExtractNumber(self, afterKeyword : str, where : str) -> int:
        index = where.find(afterKeyword)
        if index < 0: return
        index += len(afterKeyword)
        num = ""
        while where[index].isdigit():
            num += where[index]
            index += 1                
        if not num: return
        return int(num)
    
    
    def __CheckAborted(self) -> bool: 
        errors = ["Aborted!", "startup error", "Script file loading error:"]
        for e in errors:
            if self.data.find(e) >= 0:
                self.wasError = True
                self.completed = True
                return True
        
        return False
    
    def AddData(self, data : str):
        assert not self.completed
        self.data += data
                
        # aborted
        if self.__CheckAborted(): return
        
        # try get port 
        port = self.__ExtractNumber("port ", self.data)
        if port: self.ipsiusPort = port
        
        # check script completed
        scriptCompletedInd = self.data.find("Script execution's complited:")
        if scriptCompletedInd < 0: return # not yet
        
        tmp = self.data[scriptCompletedInd:]
        errCount = self.__ExtractNumber("with errors ", tmp)
        if errCount == None: return # don't have err counter yet
        
        if errCount > 0: # error
            lines = self.data.splitlines()
            lastLine = lines[len(lines) - 1]
            if (self.data.find("Script execution's complited:")
                and lastLine.startswith(":ERROR")
                and lastLine.endswith(".")):
                self.scriptCompleted = True
                self.wasError = True
                self.completed = True
            return # don't have script error description yet
        
        # ok
        self.scriptCompleted = True 
        self.completed = True      
                
    
    ScriptCompleted = Property('scriptCompleted')
    WasError        = Property('wasError')
    IpsiusOutput    = Property('data')
    Completed       = Property('completed')
    
    IpsiusPort      = Property('ipsiusPort') # int or None


# --------------------------------------------------------------
# Test
# --------------------------------------------------------------

from TestDecor import UtTest
from AllTest.TestRunner import GTestRunner


COutputErr1 = """6/16/10 2:14 PM Domain Domain started at port 58697. Heap: <no info>
Build info: "AutoBuildInfo: rev 4861; build Thu Jun 10 09:44:02 2010 by root"

Script '/home/proj/ipsius/PyIpsiusQConfig/tmp/isc/gen_IpsiusAutorun.isc' execution's started ...
> ObjCreate NObjHwFinder HwFinder true
> .LogLevel = 3
> .FirmwarePath = /home/proj/ipsius/PyIpsiusQConfig/firmware
> .Enabled = true
Exception, BfBootCli::BroadcastReceiver::BindError @ ../Src/BfBootCli/BroadcastReceiver.cpp:40 Text: BindError @ HwFinder.Enabled = true
:LOCATION /home/proj/ipsius/PyIpsiusQConfig/tmp/isc/gen_IpsiusAutorun.isc, line 5 (command # 4)
:ERROR DRI::CommandDriWriteProperty
Script execution's complited: Commands in total 4, with errors 1, service 7, scripted 0.
Script execution error: Exception, BfBootCli::BroadcastReceiver::BindError @ ../Src/BfBootCli/BroadcastReceiver.cpp:40 Text: BindError @ HwFinder.Enabled = true
:LOCATION /home/proj/ipsius/PyIpsiusQConfig/tmp/isc/gen_IpsiusAutorun.isc, line 5 (command # 4)
:ERROR DRI::CommandDriWriteProperty.""" 

COutputErr2 = """Aborted! Some error ..."""

COutputErr3 = """... startup error: invalid command line option ..."""

COutputErr4 = """6/17/10 3:26 PM Domain Domain started at port 50200. Heap: <no info>
Build info: "AutoBuildInfo: rev 4861; build Thu Jun 10 09:44:02 2010 by root"

Script file loading error: Parameter contains unquoted system symbol '/home/proj/ipsius/PyIpsiusQConfig/firm ware': "/home/proj/ipsius/PyIpsiusQConfig/firm ware" in line: .FirmwarePath = /home/proj/ipsius/PyIpsiusQConfig/firm ware.
Domain stoped. Heap: <no info>
"""

COutputOk = """6/16/10 2:18 PM Domain Domain started at port 43673. Heap: <no info>
Build info: "AutoBuildInfo: rev 4861; build Thu Jun 10 09:44:02 2010 by root"

Script '/home/proj/ipsius/PyIpsiusQConfig/tmp/isc/gen_IpsiusAutorun.isc' execution's started ...
> ObjCreate NObjHwFinder HwFinder true
> .TraceEnabled = true
> .FirmwarePath = /home/proj/ipsius/PyIpsiusQConfig/firmware
> .Enabled = true
> ObjCreate  NObjResipLog "ResipLog"
Script execution's complited: Commands in total 5, with errors 0, service 10, scripted 0."""

# --------------------------------------------------------------


@UtTest
def CoreIpsiusOutputParserTest():
    
    def Impl(outputFn):
        
        def Check(output : str, wasErr : bool, scriptCompl : bool, port : int):
            p = CoreIpsiusOutputParser()
            p.AddData(output)
            assert p.Completed
            assert p.WasError == wasErr
            assert p.ScriptCompleted == scriptCompl
            assert p.ipsiusPort == port
        
        
        Check(COutputErr1, wasErr = True,  scriptCompl = True,  port = 58697)
        Check(COutputErr2, wasErr = True,  scriptCompl = False, port = None)
        Check(COutputErr3, wasErr = True,  scriptCompl = False, port = None)
        Check(COutputErr4, wasErr = True,  scriptCompl = False, port = None)
        Check(COutputOk,   wasErr = False, scriptCompl = True,  port = 43673)
        
        outputFn("CoreIpsiusOutputParserTest: OK")
    
    GTestRunner.Run(Impl, traceTest = False)
    

# --------------------------------------------------------------


if __name__ == "__main__":
    
    import unittest
    unittest.main()


