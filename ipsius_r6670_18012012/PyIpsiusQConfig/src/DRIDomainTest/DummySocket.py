'''
Created on Apr 14, 2010

@author: root
'''
class DummySocket:
    def __init__(self, host : str, port : int,
                 traceFn : lambda str : None = None):
        self.lastRecv = b'' # return on Recv()
        
        self.sendRecv = {b'\xff\xfd\x03\xff\xfc\x01\xff\xfd\x01' 
                         : b'\xff\xfb\x03\xff\xfd\x01\xff\xfb\x01\r\nlogin: ',
                         b'1' 
                         : b'1 \x1b[D\r\npassword for 1: ',
                         b'test' 
                         : b'* \x1b[D* \x1b[D* \x1b[D* \x1b[D\r\nWelcome\
                          to domain Domain. Type "MetaTypeInfo" to begin work. \
                          Hit ESC for exit, TAB for hint.\r\nSession started \
                          14.04.2010 at 10:58:34\r\n\r\nDomain:> ',
                         b'HwFinder.ListBoards false'
                         : b'H \x1b[Dw \x1b[DF \x1b[Di \x1b[Dn \x1b[Dd \x1b[De\
                          \x1b[Dr \x1b[D. \x1b[DL \x1b[Di \x1b[Ds \x1b[Dt \x1b[DB\
                           \x1b[Do \x1b[Da \x1b[Dr \x1b[Dd \x1b[Ds \x1b[D  \x1b[Df\
                            \x1b[Da \x1b[Dl \x1b[Ds \x1b[De \x1b[D\r\nHwID = 1; \
                            HwNumber = 15; IP = "192.168.0.203"; StateDesc = \
                            "StReady"; MAC = "90:91:a1:88:00:0f"; StateEnter = \
                            14.04.2010 10:58:33; Owner = ""; BooterPort = 0; \
                            CmpPort = 4097; BootRelease = 13; BootReleaseInfo = \
                            "13/Apr  9 2010/15:03:38"; SoftRelease = 42; \
                            SoftReleaseInfo = "BfMainE1 (3544038); BfEcho \
                            (270448); "; FirstRecv = 14.04.2010 10:58:33; \
                            LastRecv = 14.04.2010 10:58:33; DiscoveredCount = 1; \
                            BroadcastCounter = 1; \r\nHwID = 1; HwNumber = 4; IP = \
                            "192.168.0.206"; StateDesc = "StReady"; MAC = \
                            "90:91:a1:88:00:04"; StateEnter = 14.04.2010 10:58:33; \
                            Owner = ""; BooterPort = 0; CmpPort = 4097; BootRelease\
                             = 13; BootReleaseInfo = "13/Apr  9 2010/15:03:38"; \
                             SoftRelease = 44; SoftReleaseInfo = "BfMainE1 (3544038); \
                             BfEcho (270448); "; FirstRecv = 14.04.2010 10:58:33; \
                             LastRecv = 14.04.2010 10:58:33; DiscoveredCount = 1; \
                             BroadcastCounter = 1; \r\nHwID = 1; HwNumber = 11; \
                             IP = "192.168.0.205"; StateDesc = "StReady"; \
                             MAC = "90:91:a1:88:00:0b"; StateEnter = \
                             14.04.2010 10:58:34; Owner = ""; BooterPort = 0; \
                             CmpPort = 4097; BootRelease = 13; BootReleaseInfo = \
                             "13/Apr  9 2010/15:03:38"; SoftRelease = 42; SoftReleaseInfo \
                             = "BfMainE1 (3544038); BfEcho (270448); "; FirstRecv = \
                             14.04.2010 10:58:34; LastRecv = 14.04.2010 10:58:34; \
                             DiscoveredCount = 1; BroadcastCounter = 1; \r\nHwID = 2; \
                             HwNumber = 121; IP = "192.168.0.202"; StateDesc = "StReady"; \
                             MAC = "90:91:a1:88:00:79"; StateEnter = 14.04.2010 10:58:34; \
                             Owner = ""; BooterPort = 0; CmpPort = 4097; BootRelease = 13; \
                             BootReleaseInfo = "13/Apr  9 2010/15:03:38"; SoftRelease = 44; \
                             SoftReleaseInfo = "BfMainE1 (3544038); BfEcho (270448); "; \
                             FirstRecv = 14.04.2010 10:58:34; LastRecv = 14.04.2010 \
                             10:58:34; DiscoveredCount = 1; BroadcastCounter = 1; \
                             \r\n:OK\r\n\r\nDomain:> ',
                             b'DomainExit' \
                             : b'\r\n:OK\r\n\r\nDomain:> '}
        
    
    def Send(self, byteData : bytes):
        if byteData == b'\r\x00': return # ignore        
        if not byteData in self.sendRecv.keys():
            print ('unknown:', byteData)
            assert 0 and "Unknown send data"            
                        
        self.lastRecv = self.sendRecv[byteData]
    
    def Recv(self, size : int) -> bytes:
        retSize = len(self.lastRecv)
        if retSize > size: retSize = size
        res = self.lastRecv[:retSize]        
        self.lastRecv = self.lastRecv[retSize:]
        return res 
        
    