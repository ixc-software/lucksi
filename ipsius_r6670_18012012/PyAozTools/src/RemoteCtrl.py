'''
Created on 29.03.2011

@author: Valkiriy
'''

from CDUtilsPack.Telnet.TelnetParams import TelnetParams
from CDUtilsPack.Telnet.TelnetClient import TelnetClient, TelnetError, TelnetInteractionProfile, TelnetConst, TelnetTimeout
from CDUtilsPack.CoreTrace import CoreTrace
    
class CtrlErr(Exception):
    pass


class RemoteCtrl:
    '''
    TelnetClient wrapper.
    For run remote control command used for Aoz service tasks
    '''           
    _PromptSfx = b'# '
    
    @staticmethod
    def _InitProfForAoz(p : TelnetInteractionProfile):
        C = TelnetConst    
        p.serverInitString = C.IAC + C.DO + C.SGA
        p.clientInitString = ( C.IAC + C.DO + C.ECHO + C.IAC + C.DO + C.NAWS + C.IAC + C.DO +
                               C.FLOWCTRL + C.IAC + C.WILL + C.ECHO + C.IAC + C.WILL + C.SGA )
        p.loginPrompt = b'login: '
        p.badLoginResponse = b'login:'        
        p.nlSfx = RemoteCtrl._PromptSfx # b'# '
        p.passPromptFn = lambda login: b'Password:'
        p.passEchoFn = lambda loginSize: None    
        p.connectTimeout = 40
    #------------------------------------------------------------------      
        
    def _FileExist(self, file : bytes) -> bool:        
        return self._ProcessCmdWithRsp(b'ls ' + file, file, 1)
    
    #------------------------------------------------------------------ 
    
    def _WaitPrompt(self, timeout = 1):  
        self.telnet.RecvUntil(RemoteCtrl._PromptSfx, timeout)
        
    #------------------------------------------------------------------
    
    def __init__(self, tracer : CoreTrace, ip : str, port : int, login, pwd = ''):                      
        
        self.Trace = lambda msg: tracer.Add('Tel', msg) 
        
        self.NLPrefix = b'\r\n[' + login.encode() # [root...                
        
        tParams = TelnetParams(ip, port, login, pwd)               
        
        prof = TelnetInteractionProfile(self._InitProfForAoz)
         
        # connect
        self.Trace('connecting to {0}:{1} as {2}...'.format(ip, port, login, pwd))
        telnetTracer = tracer.Copy()
        telnetTracer.Enable(False)        
        self.telnet = TelnetClient(tParams, telnetTracer, prof)
        self.Trace('connected.')  
        
        #------------------------------------------------------------------
             
       
    def _ProcessCmdWithRsp(self, cmd, rsp, timeout = 1, raiseCond = None):
        def procCmd():        
            try:                        
                self.telnet.Send(cmd)            
                self.telnet.RecvUntil(rsp, timeout)
            except TelnetTimeout:              
                return False
            except TelnetError as e:
                raise CtrlErr
                           
            return True
        
        def _RecErrMsg() -> str:
            timeout = 1
            try:
                b = self.telnet.RecvUntil(self.NLPrefix, timeout)                
            except TelnetTimeout:
                return ''
            b = b[:-len(self.NLPrefix)]            
            return b.decode()
        
        ok = procCmd()
        if raiseCond != None and raiseCond == ok:
            raise CtrlErr(_RecErrMsg())
        
        return ok
    
    def SDIsMount(self) -> bool:
        cmd = b'mount -t vfat'
        rsp = b'dev/sdcard on /sdcard' # todo depend CSDCardPath
        timeout = 1
        
        ok = self._ProcessCmdWithRsp(cmd, rsp, timeout)
        if ok: self._WaitPrompt()
        return ok                    
        
    
    def MountSD(self):
        #cmd = b'mount -t vfat -o rw,sync,nosuid,nodev,noatime,nodiratime,fmask=0,dmask=0,allow_utime=22,codepage=cp437,iocharset=8859-1 /dev/sdcard /sdcard'
        cmd = b'mount -t vfat -o rw,sync,nosuid,nodev,noatime,nodiratime,fmask=0,dmask=0 /dev/sdcard /sdcard'
        errRsp = b'failed:'
        timeout = 2                
        
        self._ProcessCmdWithRsp(cmd, errRsp, timeout, raiseCond = True)
                      
        
    def RestartDevice(self):
        script = b'./sdcard/ipsius/stop.sh'           
        
        if self._FileExist(script):     
            self.Trace('run stop script on device...')                      
            ok = self._ProcessCmdWithRsp(cmd = script, rsp = b'StopComplete', timeout = 30)
            if ok:
                self._WaitPrompt() 
                self.Trace('stop complete.')                
            else:
                self.Trace('stop complete waiting timeout!')
            
        self.Trace('reboot device...')        
        self._ProcessCmdWithRsp(cmd = b'shutdown -r now', rsp = RemoteCtrl._PromptSfx, timeout = 2, raiseCond = False)
        
   
        
        
            
                    
    
