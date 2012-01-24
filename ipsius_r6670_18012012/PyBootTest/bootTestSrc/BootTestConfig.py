
def _QuoteStr(s): 
    return '"' + s + '"'

CConfigVer = 1

CComNumber = 4   #  <-----

CBoardNumber = 1   #  <-----
CBoardType   = 1

CBaseMac = "90:91:92:93:94:00"

CBoardPassword = '666'
CStartupTimeout = 3000

CStaticIP = '192.168.0.102'
CDefaultIP = '192.168.0.101'

CMiscPath = r'C:\eugene_v\Hg\release''\\'   #  <-----C:\eugene_v\Hg\misc

CCmdExe = r"C:\eugene_v\Hg\trunk-hg\ProjIpsius\Debug\ProjIpsius.exe"  #  <-----

CIscPath = r"C:\eugene_v\Hg\trunk-hg\ProjIpsius\iscBootTest"   #  <-----

CCmdBaseOpt = "-ta 50200 -la 127.0.0.1:56001 -at -ae -i " + _QuoteStr(CIscPath) 

# ----------

CBooting = True
CBootIsc = 'RunMiniFlasher.isc'  # COM_number flasher_ldr booter_ldr
CFlasherLdr = CMiscPath + r'BfLoader\1001221624_bf537loader_led.ldr'

CBooterLdr  = CMiscPath + r'BfBoot\2704111810_v15.vcfg07.vfactory05.ldr'   #    <-----

# ----------

CFactoryIsc = 'SetFactory.isc'  # transport hw_type board_num mac
CViewConfigIsc = 'ViewBoardSettings.isc'  # transport password
CViewBroadcastIsc = 'ViewBroadcasts.isc'  #  board_num
CBoardConfigIsc = 'BoardConfig.isc'  # _1 .. _7
CRunNamedIsc = 'RunNamed.isc'  # transport password script_name
CUpdateBootImgIsc = 'UpdateBootImg.isc' # transport password boot_ldr

CUpdateFwIsc = 'UpdateFw.isc' # transport password fwu_file bool_check_after

CFwuHuge = CMiscPath + r'BfBoot\misc\HugeLdr.fwu'
CFwuTiny = CMiscPath + r'BfBoot\misc\SimplTestFw.fwu' 

CFwuBfMainE1 = CMiscPath + r'BfBoot\misc\MainE1v44_HWTEST.fwu'  #  <-----  compatible with CBooterLdr with named script

CNamedScript = 'HWTEST'

CUpdatedBooterLdr = CMiscPath + r'BfBoot\misc\1004151300_v00.vcfg06.vfactory05.ldr'  # Cfg version must be another





