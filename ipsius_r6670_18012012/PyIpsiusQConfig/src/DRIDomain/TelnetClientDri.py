'''
Created on Jan 25, 2010

@author: root
'''

from CDUtilsPack.MetaUtils import Property
from CDUtilsPack.CoreTrace import CoreTrace

from CDUtilsPack.Telnet.TelnetClient import (
    TelnetClient, TelnetParams, TelnetInteractionProfile, TelnetError
    )

from PackUtils.Misc import ToBytes, ToStr
from PackUtils.CoreBaseException import CoreBaseException

# --------------------------------------------------------------------

CDefOneCommandTimeoutSec = 1.0 * 60
_NL = b'\r\n'

# --------------------------------------------------------------------

class _ConstDri:
    ResultOk = ":OK"
    ResultErr = ":ERROR "
    RunScriptCmd = "CS_Exec"
    
# --------------------------------------------------------------------

class CommandDriResult:
    """Parsed result of DRI command."""
    
    def __init__(self, text : str): #, isScript : bool):
        self.raw = text
        self.ok = False
        self.text = ''
        self.err = ''
        self.errType = ''
        
        # if isScript: self.__ParseScript()
        # else: self.__ParseCmd()
        self.__ParseCmd()
        
    def __ParseError(self, toParse : str):
        nlStr = ToStr(_NL)
        self.ok = False
        self.err = toParse
        errInd = toParse.find(_ConstDri.ResultErr)
        # can be without err-sfx
        if errInd < 0:
            self.text = toParse
        else:
            self.text = toParse[0:errInd]
            skip = errInd + len(_ConstDri.ResultErr)
            self.errType = toParse[skip:].rstrip(nlStr)            
        
    def __ParseCmd(self):
        nlStr = ToStr(_NL)
        toParse = self.raw.strip(nlStr)
        
        if toParse.endswith(_ConstDri.ResultOk):
            self.ok = True
            size = len(toParse) - len(_ConstDri.ResultOk)
            self.text = toParse[:size].rstrip(nlStr)
            return
        
        self.__ParseError(toParse)        
    
#    def __ParseScript(self):
#        nlStr = ToStr(_NL)
#        toParse = self.raw.strip(nlStr)
#        
#        if not toParse.startswith(_ConstDri.ResultOk):
#            self.__ParseError(toParse)
#            return
#        
#        if not toParse.endswith(_ConstDri.ResultOk):
#            # extract last cmd
#            index = toParse.rfind(">") 
#            self.__ParseError(toParse[index:])
#            return
#        
#        self.ok = True
#        size = len(toParse) - len(_ConstDri.ResultOk)
#        self.text = toParse[:size].strip(nlStr)
    
    Raw         = Property("raw")
    OK          = Property("ok")
    Text        = Property("text")
    Error       = Property("err")    
    ErrorType   = Property("errType")

# --------------------------------------------------------------------

class DriError(CoreBaseException):
    
    def __init__(self, res : CommandDriResult, cmd : str = ""):
        text = res.Error
        if cmd: text = cmd + "\n\n" + text
        Exception.__init__(self, text)

# --------------------------------------------------------------------

class TelnetTraceStates:
    
    def __init__(self, state):
        self.ForSocket = state
        self.ForTelnet = state
        self.ForTelnetParser = state
        self.ForDri = state
        self.ForDriAsync = state
        #self.ForDriAsyncInner = state
        #self.ForDriAsynsQueue = state

# --------------------------------------------------------------------

class TelnetClientDri:
    """
    Using to send DRI commands to ProjIpsius via Telnet. Constructor and 
    Process() method can raise TelnetError, TelnetTimeout and SocketError.
    Note: be sure that you specify correct domain name 'domainName', 
    because otherwise command processing will fail on timeout (parser 
    won't find domain name in ProjIpsius output).
    """        
    
    def __init__(self, domainName : str, params : TelnetParams, trace : CoreTrace, 
                 oneCmdTimeoutSec : float = CDefOneCommandTimeoutSec):
        self.domainName = domainName
        self.trace = trace
        self.oneCmdTimeoutSec = oneCmdTimeoutSec        
        
        prof = TelnetInteractionProfile() # default
        self._NlSfx = prof.nlSfx
        
        self.telnet = TelnetClient(params, trace.Copy())       
                        
        self.cmdsWithoutRes = 0 
        
        # skip server greeting and first NL hint
        self.__SkipUntil(self._NlSfx)        
    
    _NlSfx = None
    
    
    def __Trace(self, text : str):
        self.trace.Add(self, text)        
    
    
    def __SkipUntil(self, byteData : bytes):
        res = self.telnet.RecvUntil(byteData)
        self.__Trace("Skip data: {0}".format(res))        
    
    
    def __SendCmd(self, cmd : str):
        assert cmd
        self.telnet.Send(ToBytes(cmd))
        self.cmdsWithoutRes += 1
        self.__Trace("Send: {0}".format(cmd))
        
        
    def __RecvResult(self, timeoutSec : float) -> CommandDriResult:
        """
        Read data until complete DRI command recieved.
        """
        # skip echo (ends with NL)
        self.__SkipUntil(_NL)
        
        byteData = b''
        # read until next command invitation:
        # "<NL><DomainName>:[objects]<NlSfx>"
        while (True):
            # read until next NlSfx hint
            byteData += self.telnet.RecvUntil(self._NlSfx, timeoutSec)
            nlIndex = byteData.rfind(_NL)
            if nlIndex >= 0: 
                objPath = ToStr(byteData[nlIndex + len(_NL):])                
                
                PathIsValid = objPath.startswith(self.domainName) if self.domainName\
                else objPath.find(':>') == len(objPath) - 3
                
                if PathIsValid:                    
                    # check if objPath is correct list of objects names
                    for ch in objPath:
                        if (not ch.isdigit() or not ch.isalpha() or
                            ch != '.' or ch != ':' or ch != '_'):
                            continue
                    # received next command invitation: OK
                    # cut invitation  
                    byteData = byteData[:nlIndex]
                    break
        
        self.cmdsWithoutRes -= 1
        
        return CommandDriResult(ToStr(byteData))
    
    
    def Process(self, commandList : str or [str]) -> [CommandDriResult]:
        """
        'commandList' - one command or list [] of commands.
        Send commands one by one and collect results. If command was completed with 
        error, next commands won't be sent. Return list of CommandDriResult.
        """
        results = []
        if isinstance(commandList, str): commandList = [commandList]
        
        timeoutSec = self.oneCmdTimeoutSec * len(commandList)
        
        for cmd in commandList:
            self.__SendCmd(cmd)
            res = self.__RecvResult(timeoutSec)
            results.append(res)
            if not res.OK: break
        
        return results
    

    def EnableTrace(self, states : TelnetTraceStates):
        self.trace.Enable(states.ForDri)
        self.telnet.EnableTrace(states.ForTelnet, states.ForSocket, 
                                states.ForTelnetParser)


# --------------------------------------------------------------------
# Tests
# --------------------------------------------------------------------

import DRIDomain.TelnetTest 

from TestDecor import UtTest
from AllTest.TestParams import TestParams

# --------------------------------------------------------------------

@UtTest
def TelnetClientDriTest():
    
    CTraceTest = False
    
    def Impl(p : TestParams):
        p.Output("Test TelnetClientDri started ...")
        
        _cmdTimeout = 0.5 # sec
        
        t = TelnetClientDri(p.IpsiusDomainName, p.IpsiusTelnetParams, 
                            p.CreateCoreTrace(CTraceTest), _cmdTimeout)
        
        t.EnableTrace(TelnetTraceStates(True))
        t.EnableTrace(TelnetTraceStates(False))
        
        def PrintRes(cmd, res : CommandDriResult):
            p.Output(cmd)
            p.Output("[{0}]{1}[{2}]".format(res.OK, res.Text, res.Error))
        
        def Run(cmds : str or [str], lastResOk : bool):
            results = t.Process(cmds)
            if isinstance(cmds, str): cmds = [cmds]
            
            assert len(cmds) == len(results)
            for i, cmd in enumerate(cmds):
                PrintRes("> " + cmd, results[i])
            
            last = results[len(results) - 1]
            assert last.OK == lastResOk
        
        Run("ObjList", True)
#        Run("ObjCreate NObjTestType a true", False) # don't have this type in release build
        Run(["CS_Set", 'CS_Print "1"', 'CS_Print "2"'], True)
        Run(["CS_Set", "8"], False)
        Run("ObjList", True)
        Run("DomainExit", True)
        
        p.Output("Test TelnetClientDri: OK")
        p.Complete()
        
        
    from AllTest.TestRunner import GTestRunner
    GTestRunner.RunIpsius(Impl, traceTest = CTraceTest, startEventLoop = True)


# --------------------------------------------------------------------

if __name__ == "__main__":
    
    import unittest
    unittest.main()
    
    
    
        
    
        
    
        
        
        
        