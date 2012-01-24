'''
Created on 26.12.2009

@author: Alex
'''

import sys
import os
import time

import CDUtils

# --------------------------------------------------------------

def ImportConfig():
    assert len(sys.argv) <= 2    
    cfgName = 'config' if len(sys.argv) == 1 else sys.argv[1]
    s = 'from ' + cfgName + ' import *'
    exec(s, globals())
    assert CConfigVer == 1

# --------------------------------------------------------------

class UserAbort(Exception):
    pass

def UserAsk(msg, allowSkip = True) -> bool:
    print('')
    print('DIALOG:', msg)
    s = "Press ENTER to continue, 'a' to abort"
    if allowSkip: s += ", 's' to skip"
    print(s)
    s = input()
    if s.startswith('a') : raise UserAbort()
    return not s.startswith('s')

# --------------------------------------------------------------

def OsExec(cmd, printCmd : bool) -> int:
    if printCmd: print(cmd)
    res = os.system('"' + cmd + '"')
    if printCmd: print('')
    return res
    
# --------------------------------------------------------------

class ExecError(Exception):
    pass

class ErrorMode:
    BreakOnScriptError, BreakOnScriptOk = range(2)

def RunScript(scriptName, *scriptParams, errMode : ErrorMode = ErrorMode.BreakOnScriptError):
    cmd = '"{exe}" {baseOpt} -af {scp}'.format(exe = CCmdExe, baseOpt = CCmdBaseOpt, scp = scriptName)
    
    params = ['"' + str(s) + '"' for s in scriptParams]
    if len(params) > 0: 
        cmd += ' -sp'
        for s in params:
            cmd += ' '
            cmd += s 

    retCode = OsExec(cmd, True)
    retCodeMsg = 'Return code is {0}'.format(retCode)
     
    if errMode == ErrorMode.BreakOnScriptError:
        if retCode == -1: raise ExecError(retCodeMsg)
    else:
        if retCode != -1: raise ExecError(retCodeMsg)       

# --------------------------------------------------------------

def DoAction(msg, fn, predicate = True):
    if not predicate: return
    if not UserAsk(msg): return
    fn()

# --------------------------------------------------------------

def MakeComTransport(comNum : int) -> str:
    return "COM{0}@38400".format(comNum)

# --------------------------------------------------------------

def MakeMac(baseMac : str, boardNum : int):
    return baseMac[:-2] + CDUtils.IntToHex(boardNum, 1, prefix = False)
    
# --------------------------------------------------------------

def SetFactory(transport : str):
    mac = MakeMac(CBaseMac, CBoardNumber)
    RunScript(CFactoryIsc, transport, CBoardType, CBoardNumber, mac)

# --------------------------------------------------------------

def ViewConfig(transport : str, password : str = '', errMode : ErrorMode = ErrorMode.BreakOnScriptError):
    RunScript(CViewConfigIsc, transport, password, errMode = errMode)

# --------------------------------------------------------------

def SetupConfig(transp, transpPwd, useDhcp, ip, gateway, mask, pwd, timeoutMs):
    RunScript(CBoardConfigIsc, transp, transpPwd, useDhcp, ip, gateway, mask, pwd, timeoutMs)
    
# --------------------------------------------------------------

def RunPing(addr : str):
    OsExec('ping -n 2 -w 500 ' + addr, True)
        
# --------------------------------------------------------------

def UpdateFw(transp, transpPwd, fileFwu, checkAfter : bool = False):
    
    def BoolToString(b : bool) -> str:
        return 'true' if b else 'false'
    
    RunScript(CUpdateFwIsc, transp, transpPwd, fileFwu, BoolToString(checkAfter))

# --------------------------------------------------------------

def Main():
    
    transpCom = MakeComTransport(CComNumber)
    
    # Booting and flashing
    msg = 'Boot & flash\nConnect COM, disconnect Ethernet, setup boot jumper and reset board'
    def f(): RunScript(CBootIsc, CComNumber, CFlasherLdr, CBooterLdr)
    DoAction(msg, f, CBooting)            

    # COM/Factory setup
    msg = 'COM/Factory setup\nRemove boot jumper, reset board, wait for booter started'
    def f():
        SetFactory(transpCom)
        ViewConfig(transpCom, '')
        # RunScript(CViewBroadcastIsc, CBoardNumber)
        
    DoAction(msg, f)
    
    # COM -> Enable DHCP, recv broadcast, set static IP
    # sometimes don't work as standalong - halted 
    msg = 'COM -> DHCP, static IP\nConnect Ethernet to board!'
    def f():
        SetupConfig(transpCom, '', useDhcp = True, ip = CStaticIP, gateway = '192.168.0.1', mask = '255.255.255.0', pwd = CBoardPassword, timeoutMs = CStartupTimeout)
        print('Wait...')
        time.sleep(3.0)
        ViewConfig(transpCom, '')  
                
        SetupConfig(transpCom, '', useDhcp = False, ip = CStaticIP, gateway = '192.168.0.1', mask = '255.255.255.0', pwd = CBoardPassword, timeoutMs = CStartupTimeout)
        RunPing(CStaticIP)
                          
    DoAction(msg, f)
    
    # Ethernet (static IP) -> bad password login, huge fwu, COM -> small fwu
    msg = 'IP -> bad password login, huge FWU; COM -> small FWU'
    def f():
        ViewConfig(CStaticIP, 'wrong password', ErrorMode.BreakOnScriptOk)
        
        def Huge(): UpdateFw(CStaticIP, CBoardPassword, CFwuHuge)
        
        UserAsk("Is it login error?\nPress ENTER for huge ldr upload. Abort first load by ESC", allowSkip = False)
        Huge()
        Huge()
        
        UserAsk("View: board booting. Reset board and press ENTER after", allowSkip = False)
        UpdateFw(transpCom, '', CFwuTiny)  #  <-- board halt (if huge fwu is interrupted too)
            
    DoAction(msg, f)
    
    # IP normal firmware update 
    msg = 'IP -> BfMainE1 fwu'
    def f():
        UpdateFw(CStaticIP, CBoardPassword, CFwuBfMainE1, True)  # <-- strange app startup

    DoAction(msg, f)
    
    # Run named script
    msg = 'Run script by name'
    def f():
        RunScript(CRunNamedIsc, CStaticIP, CBoardPassword, CNamedScript)
        
    DoAction(msg, f)
    
    # Booter update
    msg = 'Update booter\nDevice IP must be 192.168.0.101 after update'
    def f():
        RunScript(CUpdateBootImgIsc, CStaticIP, CBoardPassword, CUpdatedBooterLdr)
        print('Wait...')
        time.sleep(3.0)
        ViewConfig(transpCom, '')

        UserAsk("Is it default config? Is booter version changed? Is factory settings correct?", allowSkip = False)
        
        # restore static IP
        SetupConfig(transpCom, '', useDhcp = False,  ip = CStaticIP, gateway = '192.168.0.1', mask = '255.255.255.0', pwd = CBoardPassword, timeoutMs = CStartupTimeout)
                                    
    DoAction(msg, f)
    
    # Default config reset
    msg = 'Default config reset\nReset device config for continue...\nDevice IP must be 192.168.0.101'
    def f():
        RunPing(CDefaultIP)        
        ViewConfig(transpCom, '')
  
    DoAction(msg, f)

    # Done!
    print()
    print('Done!')

# --------------------------------------------------------------

""" 

    Domain: exit code on error
    ESC -- отдельный код возврата - ? реакция на ошибки при запуске - ?
    
    DRI
    SetFactoryParams -- нет логина при неправильном MACe
    Не очевидность включения DHCP    
    Сброс factory НЕ стирает конфиг -- правильно ?
    
    Halt: load BfMainE1 thru IP, after COM DHCP setup 
    
"""

# --------------------------------------------------------------

if __name__ == '__main__':
    
    ImportConfig()
    
    try:
        Main()
    except UserAbort:
        print('User abort!')
    except ExecError as e:
        print('Execution error! {0}'.format(e))
         
                  
    