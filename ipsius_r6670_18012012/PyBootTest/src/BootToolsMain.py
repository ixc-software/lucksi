'''
Created on 20.01.2010

@author: Alex
'''
import shutil

import sys
import time, datetime
import os 
import os.path
import zipfile
import tempfile
import io

import CDUtils
import BootUtils

from BooterConsts import *
from CDUtils import UserException
from CDUtils import DateTimeStamp

# --------------------------------------------

def FileInfo(name : str) -> str:
    name = name.replace('\\', '/')
    fileTime = os.path.getmtime(name)
    delta = datetime.timedelta(seconds = time.time() - fileTime)
    size = CDUtils.IntToSize(os.path.getsize(name))         
    return "{0}; size {1}; {2}; {3} days old".format(
                name, size, time.ctime(fileTime), delta.days)
    
# --------------------------------------------

def IntInputValidate(s : str) -> int:
    i = int(s)
    if i <= 0: raise Exception("Number must > 0")
    return i

def UserInput(info : str, validatorFn = lambda s: s, abort = 'a') -> int:
            
    while True:
        s = input( "{0} (type '{1}' for abort):".format(info, abort) ).strip(chr(13))
        
        if s == abort: raise UserException("User input aborted!") 
        
        try:
            res = validatorFn(s)
            return res
        except Exception as e:
            print("Error: " + str(e))

# --------------------------------------------

class OptionsMaker:
    
    # def fnOptRegister(maker : OptionsMaker, opt : CDUtils.CmdOptions)
    # def fnOptPostProcess(maker : OptionsMaker, opts : dict)
    def __init__(self, args : [str], fnOptRegister, fnOptPostProcess = None):
        
        self.optParser = CDUtils.CmdOptions(True)
                
        # register user options to parser
        self.optParser.Add( 'auto', 'Never ask user to confirm')                
        fnOptRegister(self, self.optParser) 
        
        # parse        
        self.opts = self.optParser.Parse(args, exitOnHelp = True) 

        # post process
        self.automode = False
        
        if hasattr(self, "comPortOpt"): self.__PostProcessComOpt()

        if fnOptPostProcess:
            fnOptPostProcess(self, self.opts)
        
    @property 
    def Opts(self) -> {}: return self.opts
    
    @property
    def Automode(self) -> bool:         
        return (self.automode) and ('auto' not in self.opts) 
    
    def EnableAutomode(self):
        self.automode = True
                
    def AddComOpt(self):
        
        """ add 'com' option """
        
        self.comPortOpt = True
        self.optParser.Add( 'com', 'COM port number',  ('number:int',) )
        
    def GetDefaultComPort(self):
        return BootUtils.LoadUserConfig().CComPort
        
    def __PostProcessComOpt(self):
        
        """ if 'com' option not exists -- load it from config """
        
        if 'com' not in self.opts:            
            self.opts['com'] = self.GetDefaultComPort()
            
    def SetupFileOption(self, optName, optDefaultPath, fileExt = 'ldr'):
                        
        if optName not in self.opts:
            self.automode = True
            self.opts[optName] = ""
            
        errMsg = "Can't setup file for option '{0}'".format(optName)
        self.opts[optName] = BootUtils.SmartFindFile(self.opts[optName], 
                                                     optDefaultPath, fileExt, errMsg)

# --------------------------------------------

class UserAbort(UserException):
    pass

def UserConfirm():
    s = input('Press ENTER to continue...').strip(chr(13))
    if len(s): raise UserAbort("Canceled!")        
        
# --------------------------------------------

def FlasherMain(args : [str]):
    
    def fnOptRegister(maker : OptionsMaker, opt : CDUtils.CmdOptions):
        
        maker.AddComOpt()
                   
        opt.Add( 'flasher',           'Flasher LDR file', ('ldrFile', ) )        
        opt.Add( 'image',             'Image LDR file',   ('ldrFile', ) )
        opt.Add( ('fullerase', 'fe'), 'Full flash erase' )
        
    def fnOptPostProcess(maker : OptionsMaker, opts : dict):
        
        maker.SetupFileOption('flasher', CMiniFlasherDir)
        maker.SetupFileOption('image',   CBooterDir)       
                                                    
    def RunFlasher(comNumber : int, flasherLdr : str, imageLdr : str, fullErase : bool, confirmUser : bool):
                
        # print info
        print("Port:", "COM" + str(comNumber))
        print("Flasher:", FileInfo(flasherLdr))
        print("Image:", FileInfo(imageLdr))
        print()
            
        # user ask
        if confirmUser: UserConfirm()
         
        # exec
        BootUtils.RunMiniFlasher(comNumber, flasherLdr, imageLdr, fullErase)
    
    
    maker = OptionsMaker(args, fnOptRegister, fnOptPostProcess)    
    opts = maker.Opts
                    
    RunFlasher(opts['com'], opts['flasher'], opts['image'], 'fullerase' in opts, maker.Automode)    
    

# --------------------------------------------

def ComBooterView(args : [str]):
    
    def fnOptRegister(maker : OptionsMaker, opt : CDUtils.CmdOptions):        
        maker.AddComOpt()
    
    maker = OptionsMaker(args, fnOptRegister)    
    BootUtils.RunViewSettingsCom(maker.Opts['com'])

# --------------------------------------------

def FactorySetup(args : [str]):

    def fnOptRegister(maker : OptionsMaker, opt : CDUtils.CmdOptions):
        
        maker.AddComOpt()
                   
        opt.Add( 'number',             'Device number',        ('num:int', ) )        
        opt.Add( ('hardware', 'hw'),   'Device hardware ID',   ('id:int', ) )
        
    def fnOptPostProcess(maker : OptionsMaker, opts : dict):

        def ProcessOpt(name):
            
            if name in opts: return
            opts[name] = UserInput("Input " + name, IntInputValidate)

        ProcessOpt('number')
        ProcessOpt('hardware')
                                                                                         
    maker = OptionsMaker(args, fnOptRegister, fnOptPostProcess)    
    opts = maker.Opts
    
    mac = BootUtils.MakeMac( opts['number'] )

    msg = "DevNumber: {0}; HwID: {1}; MAC {2}".format( opts['number'], opts['hardware'], mac )
    print(msg)
    
    BootUtils.SetFactoryCom( opts['com'], opts['number'], opts['hardware'], mac )

# --------------------------------------------

def NetworkSettings(args : [str]):
    
    def GatewayFromIp(ip) -> str:
        n = ip.rfind('.')
        assert n >= 0
        return ip[:n] + '.1'
    
    def fnOptRegister(maker : OptionsMaker, opt : CDUtils.CmdOptions):
        
        # transport
        opt.Add( ('tr_com', 'tcom'),   'Transport: COM port (default)',  ('number:int',) )        
        opt.Add( ('tr_ip', 'tip'),     'Transport: IP',        ('ip:str',) )
        opt.Add( ('tr_pwd', 'tpwd'),   'Transport: password',  ('pwd',) )
        
        # verify
        opt.Add('verify', 'Verify config')
        
        # options
        opt.Add( ('dhcp',),                'Option: use DHCP (default)' )
        opt.Add( ('full_ip', 'fip'),       'Option: use IP', ('ip', 'mask', 'gateway') )
        opt.Add( ('ip',),                  'Option: use IP with default mask and gateway', ('ip',) )
        opt.Add( ('password', 'pwd'),      'Option: password', ('pwd',) )
        opt.Add( ('timeout',),             'Option: timeout', ('timeout:int',) )
        
    def fnOptPostProcess(maker : OptionsMaker, opts : dict):
        
        # transport         
        if 'tr_ip' in opts: 
            P.TranspStr = opts['tr_ip']
        else:
            if 'tr_com' not in opts: 
                opts['tr_com'] = maker.GetDefaultComPort()
                maker.EnableAutomode()
            P.TranspStr = BootUtils.MakeComTransport( opts['tr_com'] )
            
        if 'tr_pwd' in opts: P.TranspPwd = opts['tr_pwd']
            
        # network dhcp vs IP        
        if 'dhcp' in opts:
            if ('full_ip' in opts) or ('ip' in opts): 
                raise UserException('Option conflict: IP vs DHCP')
            P.Dhcp = True            
            P.Ip = '0' 
            P.IpGateway = '0' 
            P.IpMask = '0' 
        else:
            if 'ip' in opts:
                P.Ip = opts['ip']
                P.IpGateway = GatewayFromIp(P.Ip)
                P.IpMask = '255.255.255.0'
            elif 'full_ip' in opts:
                t = opts['full_ip']
                P.Ip = t[0]
                P.IpGateway = t[2]
                P.IpMask = t[1]          
            else:
                P.Dhcp = True
                maker.EnableAutomode()
                                
        # misc 
        if 'password' in opts: P.Password = opts['password']
        if 'timeout' in opts:  P.Timeout = opts['timeout']
        if 'verify' in opts:   P.Verify = True    
            
    class P:
        TranspStr = ''
        TranspPwd = ''
        
        Dhcp = False
        Ip = ''  
        IpMask = ''
        IpGateway = ''
        
        Password = ''
        Timeout = 3 * 1000
        
        Verify = False        
                                                                                             
    maker = OptionsMaker(args, fnOptRegister, fnOptPostProcess)
    
    if maker.Automode:
        print( "\n".join(CDUtils.ListClassFields(P)) )
        UserConfirm()
        
    BootUtils.BoardSetup(P.TranspStr, P.TranspPwd, 
                         P.Dhcp, P.Ip, P.IpMask, P.IpGateway, P.Password, P.Timeout, P.Verify)    

# --------------------------------------------

def MakeFwuIndexFile(mainLdr, echoLdr, ver, devNumbers = '', devHwID = '1') -> str:
    
    CTempl = """
    
    Release = {ver}

    HwNumber = {dev_nums}
    HwID = {hw_id}

    Image0 = "{main}/BfMainE1"     
    {echo_img}    

    ScriptBOOT = "{echo_img_load}Load(MAIN, 0)"
        
    """
    
    CEchoImg = 'Image1 = "{echo}/BfEcho"'
    CEchoImgLoad = "Load(SPI, 1); "
    
    echo_img = ""
    echo_img_load = ""
    
    if echoLdr:
        echo_img = CEchoImg.format(echo = echoLdr)
        echo_img_load = CEchoImgLoad
         
    return CTempl.format(ver = ver, dev_nums = devNumbers, hw_id = devHwID, main = mainLdr, 
                         echo_img = echo_img, echo_img_load = echo_img_load)

# --------------------------------------------

def ExtractFwuVersion(zipFileName : str) -> int:
    
    z = zipfile.ZipFile(zipFileName, 'r')
    
    try:
    
        indexData = z.open('index.ini', mode = 'r').read()
        sl = str(indexData, 'ascii').split('\n')
        
        for s in sl:
            s = s.strip()
            n = s.find('=')
            if n < 0: continue
            if s[:n].strip() == 'Release': 
                return int( s[n+1:].strip() )
            
        raise UserException("Can't extract FWU version from " + zipFileName)
    
    finally:    
        z.close()
            
# --------------------------------------------

def MakeFirmware(args : [str]):
    
    def fnOptRegister(maker : OptionsMaker, opt : CDUtils.CmdOptions):
                
        opt.Add( 'main',             'Main LDR file',        ('file', ) )
        opt.Add( 'echo',             'Echo LDR file',        ('file', ) )
        opt.Add( 'output',           'Output FWU file',      ('file', ) )
        
        opt.Add( 'no_echo',          "Don't add echo LDR" )
        opt.Add( 'version',          "FWU file version",     ('ver:int',) )
        opt.Add( 'store',            "Disable FWU compression" )
        opt.Add( 'numbers',          "FWU target device numbers like '50-100'", ('nums',) )
        opt.Add( 'hw_id',            "FWU target device hardware ID like '1, 2'", ('ids',) )
                        
    def fnOptPostProcess(maker : OptionsMaker, opts : dict):
        
        def AbsAndVerifyPath(optName : str):            
            opts[optName] = os.path.abspath( opts[optName] )        
            if not os.path.exists( opts[optName] ): 
                raise UserException('File not found ' + opts[optName])
                    
        # main        
        if 'main' not in opts:
            maker.EnableAutomode()
            opts['main'] = os.path.join( CDUtils.GetScriptDir(), CScpToBfMainE1Ldr)
            
        AbsAndVerifyPath('main')
                
        # echo
        if 'no_echo' in opts:
            if 'echo' in opts: raise UserException('Options conflict: echo vs no_echo')
            opts['echo'] = ""
        else:
            if not 'echo' in opts: 
                opts['echo'] = os.path.join( CDUtils.GetScriptDir(), CEchoLdrFile)
                AbsAndVerifyPath('echo')
                            
        # version
        if 'version' not in opts:
            maker.EnableAutomode()            
            lastFwu = BootUtils.FindLastFileVersion(CFwuStorageDirMask)
            if not lastFwu: raise UserException("Can't autosetup version!")
            opts['version'] = ExtractFwuVersion(lastFwu[1]) + 1
                        
        # output (verify against version?)
        if 'output' not in opts:
            maker.EnableAutomode()            
            ts = DateTimeStamp( time.time() )
            fileName = "{0}_v{1}_release.fwu".format(ts, opts['version'])   # str(opts['version']) + "_release.fwu"
            opts['output'] = os.path.join(CFwuStorageDir, fileName)
            
        # numbers and ids
        if 'numbers' not in opts: opts['numbers'] = ''
        if 'hw_id'   not in opts: opts['hw_id']   = ''
                                                                                                                     
    maker = OptionsMaker(args, fnOptRegister, fnOptPostProcess)    
    opts = maker.Opts

    if maker.Automode:
        
        for i in opts:
            val = opts[i]
            if isinstance(val, bool): continue
            
            if isinstance(val, str):
                if val == "": continue
                
                # lame solution for file age info  
                if os.path.exists(val): 
                    val = "{0}; {1}; {2} days old".format(val, 
                          time.ctime(os.path.getmtime(val)),
                          CDUtils.FileDaysOld(val))
                    
            print(i + ' = ' + str(val))
            
        UserConfirm()
        
    def fileName(name): 
        return os.path.split(name)[1]
                
    def MakeIndex() -> str:
        indexFilename = os.path.join(tempfile.gettempdir(), 'index.ini')
        indexStr = MakeFwuIndexFile( fileName(opts['main']), fileName(opts['echo']), 
                        opts['version'], opts['numbers'], opts['hw_id'] )
        CDUtils.SaveStringList([indexStr, ], indexFilename)
        return indexFilename
    
    def ZipAddFile(zip, file):
        if not file: return
        zip.write( file, fileName(file) )  
        
    # make index file
    indexFilename = MakeIndex()
            
    # make fwu, zip
    compression = zipfile.ZIP_STORED if 'store' in opts else zipfile.ZIP_DEFLATED
    zip = zipfile.ZipFile(opts['output'], mode = 'w', compression = compression )
    ZipAddFile( zip, indexFilename )
    ZipAddFile( zip, opts['main'] )
    ZipAddFile( zip, opts['echo'] )
        
    zip.close()
                
# --------------------------------------------

def UploadFirmware(args : [str]):
    
    def fnOptRegister(maker : OptionsMaker, opt : CDUtils.CmdOptions):
        opt.Add( ('tr_ip', 'tip'),     'Transport: IP',        ('ip:str',) )
        opt.Add( ('tr_pwd', 'tpwd'),   'Transport: password',  ('pwd',) )
        
        opt.Add( 'file', 'FWU file', ('fileName',) )
                
    def fnOptPostProcess(maker : OptionsMaker, opts : dict):
        
        if 'tr_ip' not in opts: raise UserException("Please, specify IP!")
        
        if 'tr_pwd' not in opts:
            opts['tr_pwd'] = ''
        
        maker.SetupFileOption('file', CFwuStorageDir, 'fwu') 
                                                                                         
    maker = OptionsMaker(args, fnOptRegister, fnOptPostProcess)    
    opts = maker.Opts
    
    if maker.Automode:
        print("File to upload:", opts['file'])
        UserConfirm()
        
    BootUtils.UploadFirmware(opts['tr_ip'], opts['tr_pwd'], opts['file'])    
    
# --------------------------------------------

"""

    - functions replace class (builtin, None handles, auto restore)    
    - smart "in" and "not in" for CmdOptions dict; 
      add '*' to params: "fileName", "size:int*", "*" -- fix first opt key char '/' vs '-'
      refactoring -- class Params
      veriable params in opts dict always returned as tuple
      
    Py CmdOptios -- * notification "file, size:int*, *"
    Py -- SymbolReplace -- auto builtins, auto null stub, auto restore, correct context - ?
      
                      
    Tools:
        full flash test (led + no com info)
        view broadcasts (run HwFinder for x seconds) -- ()
    
    Add smart "manual" mode with user dialog - ?
                
    ---
    
    import into raw dict thru eval; ImportInto(module, namespace)        
                    
"""
        
# --------------------------------------------

def Main(argv : [str], debug = False):
        
    try:
        
        if len(argv) < 1: raise UserException("Mode not specified in command line!")
            
        CModes = { 
            "flasher"       : FlasherMain,
            "com_view"      : ComBooterView,
            "factory"       : FactorySetup,
            "settings"      : NetworkSettings,
            "fwu_make"      : MakeFirmware,
            "fwu_update"    : UploadFirmware,
        }
        
        mode = argv[0]
        args = argv[1:]
        
        if mode not in CModes: raise UserException("Unknown mode!")
            
        CModes[mode](args)
        
    except UserException as e:   # or just UserException
        
        if debug: raise e
         
        print(str(e))
        pressKey = not isinstance(e, UserAbort)
        if pressKey:
            print("Press ENTER to exit...")
            input()
        print()
        exit()

# --------------------------------------------
    
def TestMac():
    
    num = 1
    
    print( format(CBaseMac + num, 'x') )
    print( BootUtils.MakeMac(num) )
    
    exit()

# --------------------------------------------

def CallTest(s, a, b = None):
    c = 17
    print(CallInfo())
    exit()

# --------------------------------------------

# replace BootUtils.OsExec + print(), cover up all code
def RunAllTest(silence : bool = True):
        
    # debug
    # silence = False  
    
    def NullOsExec(cmd, printCmd):
        if printCmd: 
            print(cmd)
            print('')            
        return 0
        
    def NullPrint(*arg, **argK):
        pass
    
    # setup null funcs
    origExec = BootUtils.OsExec
    origPrint = globals()['__builtins__'].print
    
    BootUtils.OsExec = NullOsExec
    if silence: globals()['__builtins__'].print = NullPrint
    
    def Run(s : str):
        splited = CDUtils.SplitStringToCommandLineList(s, quote = '"') 
        Main( splited, debug = True ) # s.split(' ')
        
    def MakeTempFile(name, size) -> str:  # return name
        with io.open(name, 'bw') as f:
            f.write(b' ' * size)
        return name
        
    try:
        
        # run        
        Run("flasher /a")
        Run("com_view")
        Run("factory /number 1 /hw 1")
        Run("settings /a /full_ip 192.168.0.101 255.255.255.0 192.168.0.1 /pwd 123 /timeout 1000")
        
        tempFwuName = os.path.join(tempfile.gettempdir(), 'test.fwu')
        tempLdr = MakeTempFile( os.path.join(tempfile.gettempdir(), 'small.ldr'), 1024 )
        Run( 'fwu_make /a /s /no_echo /o "{0}" /m "{1}"'.format(tempFwuName, tempLdr) )
        
        Run("fwu_update /a /tip 192.168.0.1") 
        
    finally:
        
        # restore
        BootUtils.OsExec = origExec
        if silence: globals()['__builtins__'].print = origPrint
        
    pass                    
    
def Tests(withExit : bool = False):
    RunAllTest(not withExit)
    if withExit: exit()
    
def ZipTest():
    
    CFile = r'd:\Release\FWU\DevIpTdm\1003041619_v28_release.fwu' 
    z = zipfile.ZipFile(CFile, 'r')
    print( z.namelist() )
    f = z.open('index.ini', mode = 'r')
    print(f.read())
    exit()     
    
# --------------------------------------------

if __name__ == '__main__':

    Tests(False)
    
#    ZipTest()
    
    Main(sys.argv[1:])
    
    pass

