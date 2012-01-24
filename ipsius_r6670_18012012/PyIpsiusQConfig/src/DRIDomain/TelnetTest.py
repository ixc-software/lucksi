'''
Created on 25.03.2011

@author: Valkiriy
'''

from TestDecor import UtTest
from CDUtilsPack.Telnet.TelnetClient import (TelnetParams, TelnetClient, TelnetError)
from CDUtilsPack.Telnet import TelnetSocket 
from CDUtilsPack.MiscUtils import AssertForException

from AllTest.TestParams import TestParams
from PackUtils.Misc import ToStr


@UtTest
def TelnetClientTest():
    
    CTraceTest = False
    
    def Impl(p : TestParams):
            
        def CreateTelnet(port : int, login : str, password : str):
            params = TelnetParams("127.0.0.1", port, login, password)
            loginTimeoutSec = 2
            return TelnetClient(params, p.CreateCoreTrace(CTraceTest), loginTimeoutSec)
        
        port = p.IpsiusTelnetParams.Port
        
        # ok
        t = CreateTelnet(port, "1", "test")
        t.Send(b"ObjList")
        p.Output (ToStr(t.RecvUntil(b':OK')))
        t.Send(b"ObjLis")
        p.Output (ToStr(t.RecvUntil(b':ERROR')))
        t.EnableTrace(True, False, False)
        
        def TestWithErr(login : str, password : str, errText : str):
            fn = lambda:  CreateTelnet(port, login, password) 
            AssertForException(fn, TelnetError)
        
        p.Output("Test with bad login")
        TestWithErr("", "test", "Invalid login name")
        
        p.Output("Test with bad password")
        TestWithErr("1", "_test", "Invalid login name or password")
        
        p.Output("Test with empty password")
        TestWithErr("1", "", "Invalid login name or password")
        
        t.Send(b"DomainExit")
        t = None
        p.Output ("TelnetClientTest: OK")
        p.Complete()
    
    from AllTest.TestRunner import GTestRunner
    GTestRunner.RunIpsius(Impl, traceTest = CTraceTest, startEventLoop = True)
    
#---------------------------------------------------------------    
    
@UtTest
def SocketTest():
    
    CTraceTest = False
    
    def Impl(p : TestParams):
    
        def WithSocketType(socketType, checkRecvOneOf : [bytes]):
            s = TelnetSocket._BlockingSocket(p.IpsiusTelnetParams.Host, 
                                p.IpsiusTelnetParams.Port, 
                                p.CreateCoreTrace(CTraceTest))
            # send -> Ok
            s.Send(b"data")
            # recv -> Ok
            data = s.Recv(1024)
            p.Output("data = {}".format(data))
            for item in checkRecvOneOf:
                if data in item: 
                    p.Output("{}Test: OK".format(socketType.__name__))
                    return
            p.Complete()
            assert 0 and "No expected data was found."
        
        checkRecv = b'\xff\xfb\x03\xff\xfd\x01\xff\xfb\x01'
        
        WithSocketType(TelnetSocket._BlockingSocket, [checkRecv])
        WithSocketType(TelnetSocket._NonBlockingSocket, [checkRecv, b''])
        
        p.Complete()
    
    from AllTest.TestRunner import GTestRunner
    GTestRunner.RunIpsius(Impl, traceTest = CTraceTest, startEventLoop = True)
    
#---------------------------------------------------------------
    
if __name__ == '__main__':
    
    import unittest
    unittest.main()

