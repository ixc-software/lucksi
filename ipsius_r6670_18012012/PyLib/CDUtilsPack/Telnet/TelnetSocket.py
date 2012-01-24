'''
Created on Jan 27, 2010

@author: root
'''
from CDUtilsPack.MiscUtils import AssertForException
from CDUtilsPack.CoreTrace import CoreTrace
from CDUtilsPack.Telnet.TelnetParams import TelnetParams

import socket
import time

# --------------------------------------------------------------------

class SocketError(Exception):
    
    def __init__(self, desc : str, err : Exception):
        Exception.__init__(self, "{0}: {1}".format(desc, err))

# --------------------------------------------------------------------

class SocketTimeout(SocketError):
    
    def __init__(self, desc : str, err : Exception):
        SocketError.__init__(self, desc, err)

# --------------------------------------------------------------------

def _TryExcecute(socketAction, desc : str) -> bytes or None:
    """socketAction = fn() -> bytes or None"""
    res = None
    try:
        res = socketAction()
    except socket.timeout as t:
        raise SocketTimeout(desc, t)
    except socket.error as e:
        raise SocketError(desc, e)
    
    return res
        
# --------------------------------------------------------------------

class _NonBlockingSocket:
    """Non-blocking socket wrapper."""
    
    def __init__(self, host : str, port : int, trace : CoreTrace): # raise SocketError
        """Try create connection. If fail, raise SocketError."""
        assert isinstance(trace, CoreTrace)
        self.trace = trace
        self.socket = None 
        
        def Create():
            self.socket = socket.create_connection((host, port), None)
        
        _TryExcecute(Create, "Create")
    
        # set non-block
        self.socket.setblocking(0)
        self.__Trace("Connected.")
    
    
    def __Trace(self, text : str):
        self.trace.Add(self, text)
        
    
    def Send(self, byteData : bytes): # raise SocketError
        """Try send all data. If fail, raise SocketError."""
        assert self.socket
        
        # non-blocking
        fn = lambda: self.socket.sendall(byteData)
        _TryExcecute(fn, "Send")
        self.__Trace("Send: {}".format(byteData))
            
    
    def Recv(self, size : int) -> bytes: # raise SocketError
        """
        Try receive requested amount of data. If failed on error #11 on 
        Linux and #10035 on Win32 (what means that there is no data to 
        read immediately), return empty byte array. For any other error 
        or if recv() return no data (what means that connection was closed) 
        raise SocketError.  
        """
        assert self.socket
        
        # non-blocking
        try:
            res = self.socket.recv(size)
        except socket.error as err:
            if err.errno == 11: # err if no data to read immediately
                return b""
            if err.errno == 10035: # err if no data to read immediately
                return b""
            raise SocketError("Recv", err)
        except socket.timeout as t:
            raise SocketTimeout("Recv", t)
        else:
            if not res: self.__Error("Connection closed by foreign host.")
            self.__Trace("Recv: {}".format(res))
            return res
    
    
    def EnableTrace(self, state : bool):
        self.trace.Enable(state) 


# --------------------------------------------------------------------

CDefaultSocketTimeoutSec = 30.0

# --------------------------------------------------------------------

class _BlockingSocket:
    """Blocking socket wrapper."""
    
    def __init__(self, host : str, port : int, trace : CoreTrace, timeout): # raise SocketError
        """Try create connection. If fail, raise SocketError."""
        assert isinstance(trace, CoreTrace)
        self.trace = trace
        self.socket = None
        
#        def Create():
#            self.socket = socket.create_connection((host, port), timeout)
            
        # Fix connect with timeout
        def Connect():
            ts = int(time.time())
            def connect():
                try:
                    self.socket = socket.create_connection((host, port))
                    return True                                      
                except socket.error as e:
                    if int(time.time()) - ts >= timeout: raise e
                    return False
                                
            while not connect(): pass
            
         
        _TryExcecute(Connect, "Connect")
        
        # set blocking
        self.socket.setblocking(1) 
        self.__SetTimeout(CDefaultSocketTimeoutSec)
        self.__Trace("Connected.")
        
    
    def __Trace(self, text : str):
        self.trace.Add(self, text)
    
    
    def __SetTimeout(self, t : float):
        assert t == None or t > 0
        if self.socket.gettimeout() == t: return
        self.socket.settimeout(t) # set send/recv timeout
    
    
    def __CheckSocket(self):
        assert self.socket
    
    
    def Send(self, byteData : bytes, 
             timeoutSec : float = CDefaultSocketTimeoutSec): # raise SocketError
        """Try send all data. Raise SocketError on timeout."""
        self.__CheckSocket()
        
        def Impl():
            self.__SetTimeout(timeoutSec)
            self.socket.sendall(byteData)
        
        _TryExcecute(Impl, "Send")
        self.__Trace("Send: {}".format(byteData))
        
    
    def Recv(self, size : int, 
             timeoutSec : float = CDefaultSocketTimeoutSec) -> bytes: # raise SocketError
        """
        Try receive requested amount of data. On timeout raise SocketError.
        """
        self.__CheckSocket()
        
        def Impl() -> bytes:
            self.__SetTimeout(timeoutSec)
            return self.socket.recv(size)
        
        res = _TryExcecute(Impl, "Recv")
        assert res != None
        self.__Trace("Recv: {}".format(res))
        return res                               
    
    def EnableTrace(self, state : bool):
        self.trace.Enable(state) 
    
# --------------------------------------------------------------------

Socket = _BlockingSocket        
