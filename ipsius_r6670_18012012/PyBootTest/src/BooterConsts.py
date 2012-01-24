
import os
import os.path

_join = os.path.join

# misc
_CReleaseDir = os.getenv('IPSIUS_RELEASE')
assert _CReleaseDir

# dir where mini-flasher ldr files placed
CMiniFlasherDir     = _join(_CReleaseDir, 'BfLoader')
CMiniFlasherDirMask = _join(CMiniFlasherDir, '*.ldr')
 
# dir where booter ldr files placed
CBooterDir     = _join(_CReleaseDir, 'BfBoot')
CBooterDirMask = _join(CBooterDir, '*.ldr')

# dir with echo ldr 
CEchoLdrFile = "../../Blackfin/BfEcho/Release/vdk_echo_packed.ldr"  # must be packed - ?   

# dir with Win32 booter exe
CWin32BooterDir      = _join(_CReleaseDir, 'PcBootTool/win32')  # linux support - ?
CWin32BooterDirMask  = _join(CWin32BooterDir, '*.exe')

CBooterBaseOpt = "-la 127.0.0.1:56001 -at -ae"

# script to BfMainE1 path
CScpToBfMainE1Ldr = '../../Blackfin/BfMainE1/Release/BfMainE1_packed.ldr'

# FWU path
CFwuStorageDir     = _join(_CReleaseDir, "FWU/DevIpTdm") 
CFwuStorageDirMask = _join(CFwuStorageDir, "*.fwu") 

# Base MAC address
CBaseMac = 0x9091a1880000
