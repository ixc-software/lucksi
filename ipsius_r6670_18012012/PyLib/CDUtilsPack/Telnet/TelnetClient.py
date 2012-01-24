'''
Created on Jan 21, 2010

@author: root
'''

import time

from CDUtilsPack.CoreTrace import CoreTrace
from CDUtilsPack.MetaUtils import Property

from CDUtilsPack.Telnet.TelnetSocket import Socket, SocketError, SocketTimeout
from CDUtilsPack.Telnet.TelnetParams import TelnetParams

from CDUtilsPack.StrUtils import ToBytes
#from CDUtilsPack.CheckFnArgs import CheckFnArgs
import types 

# --------------------------------------------------------------------

class TelnetInteractionProfile: pass # fwd declaration

class TelnetConst:
    
    IAC  = bytes([255])
    DONT = bytes([254])
    DO   = bytes([253])
    WONT = bytes([252])
    WILL = bytes([251])
    SB =  bytes([250])
    SE  = bytes([240])
        
    ECHO = bytes([1])
    SGA = bytes([3])
    
    NAWS = bytes([31])
    FLOWCTRL = bytes([33]) 
    
    DblIAC = IAC + IAC # all IACs in user strings replaces by double IACs      
    
    # Timeouts 
    DefSendRecvTimeoutSec = 60.0
            
    # Read
    DefReadSize = 1024   
    
    ClientNewLine = b'\x0d\x00'
        
# --------------------------------------------------------------------

def InitProfForIpsiusServer(p : TelnetInteractionProfile):    
    C = TelnetConst
    p.serverInitString = C.IAC + C.DO + C.SGA + C.IAC + C.WONT + C.ECHO + C.IAC + C.DO + C.ECHO
    p.clientInitString = C.IAC + C.WILL + C.SGA + C.IAC + C.DO + C.ECHO + C.IAC + C.WILL + C.ECHO  
    p.loginPrompt = b'login: '
    p.badLoginResponse = b'Bad login'           
    p.nlSfx = b'> '
    
    p.passPromptFn = lambda login: b'password for ' + login + b': '
    
    CPasswordEchoChar = b'*'
    CServerEcho = b'\x20\x1b\x5b\x44'
    #p.passEchoFn = lambda loginSize: CPasswordEchoChar * loginSize + CServerEcho    
    p.passEchoFn = lambda loginSize: (CPasswordEchoChar + CServerEcho) * loginSize    

# --------------------------------------------------------------------

class TelnetInteractionProfile:
    '''
    Settings of interaction with server          
    '''          
    def __init__(self, setSpecFn = InitProfForIpsiusServer):
        # Default settings
        self.cmdLen = 3        
        self.loginTimeoutSec = 30.0
        self.connectTimeout = 10.0
        
        # Set server specific settings
        setSpecFn(self)
        
        # Chek result of setSpecFn
        self._IsComplete()
        
    def _IsComplete(self):        
        def AssertInit(val): assert(val != None)
        
        def AssertType(obj, type):
            AssertInit(obj) 
            assert(isinstance(obj, type))                
        
        AssertType(self.serverInitString, bytes)            #IAC + DO + ...
        AssertType(self.clientInitString, bytes)            #IAC + DO + ...
        AssertType(self.loginPrompt, bytes)
        AssertType(self.badLoginResponse, bytes)            
        AssertType(self.nlSfx, bytes)                                   #system prompt
        AssertType(self.passPromptFn, types.FunctionType)    #Fn(login : str)->bytes
        AssertType(self.passEchoFn, types.FunctionType)     #Fn(loginSize : int)       

# --------------------------------------------------------------------

class TelnetError(Exception): 
    
    def __init__(self, text : str):
        Exception.__init__(self, text)

# --------------------------------------------------------------------

class TelnetTimeout(TelnetError): 
    
    def __init__(self, action : str):
        text = "{0}: Timeout!".format(action)
        TelnetError.__init__(self, text)

# --------------------------------------------------------------------    

class _TelnetParser:
    """
    Parsing data receiving via Telnet, split it to user data and 
    telnet commands.
    """
    def __init__(self, trace : CoreTrace):
        self.userData = bytearray()
        self.currCmd = bytearray()
        self.currSb = bytearray()
        self.unparsed = bytearray()
        self.trace = trace
    
    
    def __Trace(self, text : str):
        self.trace.Add(self, text)
    
    
    def __ProcessCmd(self, byte : int):
        """Process IAC <cmd> <opt> (just skip it for now)."""
        assert not self.currSb
        self.currCmd.append(byte)
        if len(self.currCmd) == self.prof.cmdLen:
            self.currCmd = b''
    
        
    def __ProcessSb(self, byte : int):
        """Process IAC SB ... IAC SE (just skip it for now)."""
        assert not self.currCmd
        self.currSb.append(byte)
        if byte ==  TelnetConst.SE and self.currSb[-1] == TelnetConst.IAC:
            self.currSb = b''
    
    
    def __ProcessUserData(self, byte : int):
        """Process user data."""
        assert not self.currCmd and not self.currSb
        self.userData.append(byte)
    
    
    def Parse(self, byteData : bytes) -> bytes:
        """
        Parse given byte array and return user data. Save incomplete 
        Telnet commands to buffers.
        """
        toParse = self.unparsed + byteData
        self.__Trace("Parse: {0}".format(byteData))
        self.unparsed = bytearray()        
        i = 0
        while i < len(toParse):
            curr = toParse[i]
            i += 1
            if curr != TelnetConst.IAC: 
                if self.currCmd: self.__ProcessCmd(curr)
                elif self.currSb: self.__ProcessSb(curr)
                else: self.__ProcessUserData(curr)
                continue
            
            # IAC
            if i >= len(toParse):      # single IAC
                if self.currSb: self.__ProcessSb(curr)
                else: self.unparsed = curr
                break
            
            # after IAC
            next = toParse[i]
            i += 1
            if next == TelnetConst.SB:       # SB begin
                self.__ProcessSb(curr)
                self.__ProcessSb(next)
            elif next == TelnetConst.IAC:    # data
                if self.currSb: self.__ProcessSb(next)
                else: 
                    self.__ProcessUserData(curr)
                    self.__ProcessUserData(next)
            else:                       # cmd
                self.__ProcessCmd(curr)
                self.__ProcessCmd(next)            
        
        res = self.userData
        self.userData = bytearray()
        self.__Trace("Parsed result: {0}".format(res))
        return res
    
    
    def EnableTrace(self, state : bool):
        self.trace.Enable(state)
    
# --------------------------------------------------------------------

class TelnetClient:
    """ 
    Establish connection with the Telnet-server and ignore any further 
    Telnet-commands. Constructor, send and receive methods can raise 
    TelnetError, TelnetTimeout and SocketError. 
    """
    
    def __init__(self, params : TelnetParams, trace : CoreTrace, prof = TelnetInteractionProfile()):                       
      
        self.buff = b''
        self.parser = _TelnetParser(trace.Copy())
        self.trace = trace
        self.prof = prof
        self.socket = Socket(params.Host, params.Port, trace.Copy(), self.prof.connectTimeout) # connected
        
        
        def Establish(): 
            """"Establish connection with the server."""
            self.__Trace("Establishing ... ")
            self.socket.Send(prof.serverInitString, prof.loginTimeoutSec)
            
            try:                
                self.__RecvUntilRaw(prof.clientInitString, prof.loginTimeoutSec)                
            except TelnetTimeout:
                self.__Error("Establish: Failed")
                
            self.__Trace("Establish: OK")
        
        Establish()        
        self.__Login(ToBytes(params.Login), ToBytes(params.Password), prof.loginTimeoutSec)
    
    def __IsBadLoginOrPassword(self) -> bool:
        # try find bad login promt in received data
        res = True
        try:
            data = self.__RecvUntilRaw(self.prof.badLoginResponse, 1)
        except TelnetTimeout:
            res = False
        self.__Trace("Bad login: {}".format(res))
        return res
    
    
    def __SendLogin(self, login : str, loginTimeoutSec : float):
        """
        Send login-name and wait for password promt. Raise TelnetError 
        if failed.
        """
        self.__Trace("Send login: {}".format(login));
        self.__RecvUntilRaw(self.prof.loginPrompt, loginTimeoutSec) 
        self.Send(login)
        # try read password promt
        passwordPromt = self.prof.passPromptFn(login)
        try:
            self.__RecvUntilRaw(passwordPromt, loginTimeoutSec)
        except TelnetTimeout as e:
            if not self.__IsBadLoginOrPassword():
                raise e
            self.__Error("Invalid login name.")
    
    
    def __SendPassword(self, password : str, passwordTimeoutSec : float):
        """Send password. Raise TelnetError if receive bad login promt."""
        self.__Trace("Send password: {}".format(password))
        self.Send(password)
        
        # wait password echo
        passwordEcho = self.prof.passEchoFn(len(password))
        if passwordEcho: 
            self.__RecvUntilRaw(passwordEcho, passwordTimeoutSec)
        
        # try read further data
        try:
            data = self.__RecvUntilRaw(self.prof.nlSfx,
                                       passwordTimeoutSec)            
        except TelnetTimeout as waitDomainPromtTimeout:
            if not self.__IsBadLoginOrPassword(): 
                raise waitDomainPromtTimeout
            self.__Error("Invalid login name or password.")
        else:
            # return read to the buffer
            tail = self.buff 
            self.buff = data + tail
            
    
    def __Login(self, login : str, password : str, loginTimeoutSec : float):
        self.__Trace("Start logging in ...".format(login, password))
        self.__SendLogin(login, loginTimeoutSec)        
        self.__SendPassword(password, loginTimeoutSec)
        self.__Trace("Logged in.")
    
    
    def __Error(self, text : str):
        raise TelnetError(text)
    
    
    def __Trace(self, text : str):
        self.trace.Add(self, text)
        
    
    def __ReadToBuff(self, size : int, timeoutSec : float):
        """"Read available amount of data that less or equal to 'size'."""
        assert size > 0        
        
        data = self.socket.Recv(size, timeoutSec)
        assert(isinstance(data, bytes))
        self.buff += data
        
        bufStr = ''
        for i in range(0, len(data)): bufStr += '{} '.format(data[i]) 
        self.__Trace("ReadToBuff: {}".format(bufStr))
            
    
#    def __RecvUntilRaw(self, byteData : bytes, 
#                       timeoutSec : float = _Const.DefRecvUntilTimeoutSec) -> bytes:
#        """Receive until 'byteData' will be found."""
#        
#        def CheckTimeout(start : float, timeout : float) -> bool:
#            assert start
#            if timeout is None: return False
#            
#            return (time.time() - start) >= timeout
#        
#        def GetFromBufferUntil(byteData : bytes) -> bytes:
#            """
#            Search for the first appearance of 'byteArray' in buffer. If it's 
#            found, return all data before 'byteData' and 'byteData' itself. 
#            Otherwise return empty byte array.
#            """
#            assert byteData
#            
#            size = len(byteData)
#            foundInd = self.buff.find(byteData)
#            if foundInd < 0: return b''
#            
#            foundInd += size
#            res = self.buff[:foundInd]
#            self.buff = self.buff[foundInd:]
#            
#            return res   
#        
#        self.__Trace("Receive until: {0}".format(byteData))
#        size = len(byteData)
#        res = GetFromBufferUntil(byteData)
#        if res: return res
#        
#        startTime = time.time()
#        while True:
#            self.__ReadToBuff(size)
#            res = GetFromBufferUntil(byteData)
#            if res: break
#            
#            #time.sleep(0.01)
#            
#            if CheckTimeout(startTime, timeoutSec): 
#                raise TelnetTimeout("Receive until {}".format(byteData))
#        
#        self.__Trace("Receive until completed.")
#        return res
    
    
    def __GetFromBufferUntil(self, byteData : bytes) -> bytes:
        """
        Search for the first appearance of 'byteArray' in buffer. If it's 
        found, return all data before 'byteData' and 'byteData' itself. 
        Otherwise return empty byte array.
        """
        assert byteData
        
        size = len(byteData)
        foundInd = self.buff.find(byteData)
        if foundInd < 0: 
            return b''
        
        foundInd += size
        res = self.buff[:foundInd]
        self.buff = self.buff[foundInd:]
        
        return res  
    
    
    def __RecvUntilRaw(self, byteData : bytes, timeoutSec : float) -> bytes:
        """Receive until 'byteData' will be found."""
        assert timeoutSec > 0
        self.__Trace("Receive until: {0}".format(byteData))
        size = len(byteData)
        
        # try find in received earlier 
        res = self.__GetFromBufferUntil(byteData)
        if res: return res
        
        # try read from socket, raise SocketError on timeout
        try:
            while True:
                self.__ReadToBuff(size, timeoutSec)
                res = self.__GetFromBufferUntil(byteData)
                if res: 
                    self.__Trace("Receive until completed.")
                    return res
        
        except SocketTimeout:
            raise TelnetTimeout("Receive until {}".format(byteData))
        
    
    def __Parse(self, byteData : bytes) -> bytes:
        """"
        Parse received from socket data. Ignore Telnet-specific data 
        and return user data. Replace double IAC by IAC.
        """        
        res = self.parser.Parse(byteData)
        # replace back IAC IAC by IAC
        if TelnetConst.DblIAC in res:
            res = res.replace(TelnetConst.DblIAC, TelnetConst.IAC)
            
        return res
    
        
    def RecvUntil(self, byteData : bytes, 
                  timeoutSec : float = TelnetConst.DefSendRecvTimeoutSec) -> bytes:
        """
        Receive until 'byteData' will be found. Ignore Telnet commands.
        Can raise TelnetError.
        """
        res = self.__RecvUntilRaw(byteData, timeoutSec)
        return self.__Parse(res)
    
    
    def Recv(self, size : int = TelnetConst.DefReadSize,
             timeoutSec : float = TelnetConst.DefSendRecvTimeoutSec) -> bytes:
        """Receive. Ignore Telnet commands. Can raise TelnetError."""
        self.__ReadToBuff(size, timeoutSec)
        byteData = self.buff       
        self.buff = b'' 
        return self.__Parse(byteData)
    
    
    def Send(self, byteData : bytes, eof : bool = True,
             timeoutSec : float = TelnetConst.DefSendRecvTimeoutSec):
        """Send. Replace IAC by double IAC. Can raise TelnetError."""
        if TelnetConst.IAC in byteData:
            byteData = byteData.replace(TelnetConst.IAC, TelnetConst.DblIAC)
        self.socket.Send(byteData, timeoutSec)
        if eof: self.socket.Send(TelnetConst.ClientNewLine, timeoutSec)
          
    
    def EnableTrace(self, forSelf : bool, forSocket : bool, forParser : bool):
        self.trace.Enable(forSelf)
        self.parser.EnableTrace(forParser)
        self.socket.EnableTrace(forSocket)        
    

        
        
    
    
    
    


