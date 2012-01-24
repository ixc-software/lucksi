'''
Created on 28.03.2011

@author: Valkiriy
'''

import os
from RemoteCtrl import RemoteCtrl
from Ftp import Ftp


# todo move up to utils
def _InsertEnvVarsInPath(path : str) -> str:    
    
    CMarker = '%'
    
    res = path

    while True:
        posBegin = res.find(CMarker)
        if posBegin < 0: break
        posEnd = res.find(CMarker, posBegin + 1)
        assert posEnd >= 0, "Bad env in path " + path
        
        env = res[posBegin + 1:posEnd]
        envVal = os.getenv(env)
        assert envVal is not None, "Can't find env {0} in path {1}".format(env, path)
        res = res[:posBegin] + envVal + res[posEnd + 1:]
        
    res = res.replace('\\', '/')
                            
    return res

# -------------------------------------------------------------------- 

def _FixPathParam(path):                       
    if path.startswith("'") and path.endswith("'"): path = path[1:-1] # remove ''
    return _InsertEnvVarsInPath(path)     

# -------------------------------------------------------------------- 

def _CmdLineParser(cmdLine : [str]) -> {}:
    from CDUtilsPack.MetaUtils import UserException    
    
    cmdLine = cmdLine[1:]        
    from CDUtils import CmdOptions
    
    opts = CmdOptions(autoAddShortNames = True)   

    # Required params: 
    opts.Add('ip', "Host address of device", ('ip:str',), req = True)      
    opts.Add('telnet', "Telnet connection params", ('port:int', 'login:str', 'pwd:str*'), req = True)    
    opts.Add('ftp', "FTP connection params",  ('port:int', 'login:str', 'pwd:str*'), req = True)
    requiredCount = 3
    
    # Commands:
    opts.Add('upl', 'Upload from src (path|concrete file)', ('src:str', 'forced*') )
    opts.Add('network', 'Set network settings', ('ip:str', 'mask:str', 'gateway:str', 'dns:str') )
    opts.Add('dwl', 'Download SD card content and save it in zip-archive dst', ('dst:str', ) )
    
    result = opts.Parse( cmdLine, exitOnHelp = True)
    
    # Command must be one
    if len(result) > requiredCount + 1: raise UserException('Must be only one command!')
    
    #fix params
    if 'upl' in result:
        def Fix(src, forced = False):
            src = _FixPathParam(src)
            return src, forced
        
        result['upl'] = Fix(*result['upl'])
            
    # tPort = 23
    # ftpPort = 21
    
    return result

# --------------------------------------------------------------------    

def _Run(argv):
    from CDUtilsPack.MetaUtils import UserException    
    from CDUtilsPack.CoreTrace import CoreTrace    
    
    def traceOut(obj, text : str, textId : str):
        print(obj, text)
            
    tracer = CoreTrace( traceOut, True )
    trace = lambda msg: tracer.Add('', msg) 
        
    try:     
        # parse
        sett = _CmdLineParser(argv)      
    
        ctrl = RemoteCtrl(tracer, sett['ip'], *sett['telnet'])
        isMount = ctrl.SDIsMount()        
        
        if not isMount:
            trace('SD card not mounted. Do mount...') 
            ctrl.MountSD()        
        
        trace('SD card mounted.')
        assert( ctrl.SDIsMount() )                 
        
        ftp = Ftp(tracer, sett['ip'], *sett['ftp'])        
        
        if 'upl' in sett:
            ftp.Upload(*sett['upl'])          
                          
        if 'network' in sett:
            ftp.SetNetwork(*sett['network'])
            
        if 'dwl' in sett:            
            ftp.Download(sett['dwl']) 
            
        ctrl.RestartDevice()                     
            
                   
    except UserException as e:
        print ("Aborted!\nStart-up error:", e)            
#    except Exception as e:
#        print("Aborted!\n{0}: {1}".format(type(e).__name__, e))
    else:
        print('Script ', argv[0], ' complete successfully.')
        
# --------------------------------------------------------------------        

#/ip 192.168.0.220 /t 23 root 1  /ftp 21 admin admin /upl %IPSIUS_RELEASE%\IpsiusPacks\linux_arm\
#/network 192.168.0.220 255.255.255.0 192.168.0.32 192.168.0.32

if __name__ == '__main__':    
    import sys             
    from CDUtilsPack.MiscUtils import DirUp, GetScriptDir
    sys.path.append( DirUp(GetScriptDir(), 1) )       
   
    _Run(sys.argv)    
        
