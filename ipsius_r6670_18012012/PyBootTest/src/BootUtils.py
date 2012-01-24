'''
Created on 20.01.2010

@author: Alex
'''

import os, os.path
import glob

import CDUtils
from CDUtils import UserException
from CDUtilsPack import MiscUtils
from BooterConsts import *

# -------------------------------------------

def LoadUserConfig() -> object:
    
    def IdentStringList(sl : [str], identLevel : int) -> [str]:
        ident = identLevel * '\t'
        return [ident + s.strip() for s in sl] 
    
    cfgName = os.path.join(CDUtils.GetScriptDir(), '../cfg/config.py')
    cfg = CDUtils.LoadStringList(cfgName)
    
    cfgSrc = 'class Config:\n' + '\n'.join( IdentStringList(cfg, 1) ) + '\n'
    
#    print(cfgSrc)
    exec(cfgSrc)
    
    cfg = eval("Config()")
    assert cfg.CVersion == 1
    
    return cfg

# -------------------------------------------

def FindLastFileVersion(searchPattern : str) -> (int, str) or None:

    """ Return (file revision, file name) """
    
    def GetFileRevision(f : str) -> int or None:
        name = os.path.split(f)[1]
        n = name.find('_')
        try:
            return int(name[:n])
        except:
            return None
    
    return MiscUtils.FindLastFileRevision(searchPattern, GetFileRevision)

# -------------------------------------------

def SmartFindFile(fileName, defaultDir, fileType, errMsg) -> str:
    
    def Find() -> str or None:
                
        # fileName not empty                
        if fileName:  
                        
            if os.path.isabs(fileName): return fileName    # path is absolute                        
            if os.path.exists(fileName): return fileName   # exitsts in current dir
            
            # try to find in defaultDir
            name = os.path.join(defaultDir, fileName)
            if os.path.exists(name): return name
            
            # not found (fileName was relative)
            return None   

        # fileName is empty, try to auto find in defaultDir
        name = os.path.join(defaultDir, '*.' + fileType)
        res = FindLastFileVersion(name)
        return None if not res else res[1]
    
    if not os.path.exists(defaultDir): 
        raise UserException( "Path {0} not exists".format(defaultDir) )
            
    result = Find()
    
    if fileName: errMsg += "; file " + fileName
    
    if not result: raise UserException(errMsg)
    
    result = os.path.abspath(result)  
    if not os.path.exists(result): raise UserException(errMsg)
        
    return result

# -------------------------------------------

def _RunScript(scriptName : str, *scriptParams) -> int:
    
    exeFile = FindLastFileVersion(CWin32BooterDirMask)
    if not exeFile: raise UserException("Booter EXE not found!")
    
    scpPath = os.path.join(CDUtils.GetScriptDir(), '../isc/')
    scpPath = os.path.abspath(scpPath)
    
    cmd = '"{exe}" {baseOpt} -i "{scpPath}" -af {scp}'.format(exe = exeFile[1], 
            baseOpt = CBooterBaseOpt, scp = scriptName, scpPath = scpPath)
    
    params = ['"' + str(s) + '"' for s in scriptParams]
    if len(params) > 0: 
        cmd += ' -sp'
        for s in params:
            cmd += ' '
            cmd += s 

    retCode = OsExec(cmd, True)
    
    if retCode != 0: 
        raise UserException("Exec fail! Code " + str(retCode))        
    
    return retCode

# -------------------------------------------
        
def OsExec(cmd, printCmd : bool) -> int:
    if printCmd: print(cmd)
    res = os.system('"' + cmd + '"')
    if printCmd: print('')
    return res

# -------------------------------------------

def RunMiniFlasher(comPort : int, loaderFile, imageFile, flashErase : bool):
    scp = 'RunMiniFlasherFullErase.isc' if flashErase else 'RunMiniFlasher.isc' 
    _RunScript(scp, comPort, loaderFile, imageFile)

# -------------------------------------------

def BoolToStr(b : bool) -> str:
    assert isinstance(b, bool)
    if b: return "true"
    return "false"

# -------------------------------------------

def MakeComTransport(comNum : int) -> str:
    return "COM{0}@38400".format(comNum)

def _RunViewSettings(transp : str, password : str = ''):
    _RunScript('ViewBoardSettings.isc', transp, password)
    
def RunViewSettingsCom(comNum : int, password : str = ''):
    _RunViewSettings( MakeComTransport(comNum), password )

def SetFactoryCom(comNum : int, devNum : int, hwID : int, mac : str):
    _RunScript('SetFactory.isc', MakeComTransport(comNum), hwID, devNum, mac)

# -------------------------------------------
    
def BoardSetup(transpStr, transpPwd, useDhcp : bool, ip, ipMask, ipGateway, 
               usePassword, timeout, doVerify : bool):
    
    print( CDUtils.CallInfo() )

    """
    // _1 -> transport
    // _2 -> transport password
    
    // settings:
    // _3 -> useDhcp
    // _4, _5, _6 -> ip, gateway, mask
    // _7 -> password
    // _8 -> wait time (ms)
    // _9 -> verify
    """
        
    _RunScript('BoardConfig.isc', transpStr, transpPwd, BoolToStr(useDhcp), ip, ipGateway, ipMask,
               usePassword, timeout, BoolToStr(doVerify) )
    
# -------------------------------------------

def UploadFirmware(transpStr, transpPwd, file):
    _RunScript( 'UpdateFw.isc', transpStr, transpPwd, file, BoolToStr(True) )
    
# -------------------------------------------
    
def MakeMac(devNumber : int) -> str:        
    s = CDUtils.IntToHex(CBaseMac + devNumber, prefix = False)
    assert len(s) == 12
    pairs = []
    for i in range(6):
        offs = i * 2 
        pairs.append( s[offs:offs + 2] )
    return ":".join(pairs)
                        
# -------------------------------------------

if __name__ == '__main__':
    
#    print(dir(LoadUserConfig()))

    print( FindLastFileVersion(r"d:\proj\Ipsius misc\Ipsius\BfBoot\*.ldr") )
    
    pass
