'''
Created on Jun 8, 2010

@author: root
'''

import os.path
import os
import sys
import shutil
import datetime
import time
import glob
import zipfile
import tempfile


from InstallDesc import InstallDesc, InsertEnvVarsInPath

import CDUtils
from CDUtils import UserException
from CDUtils import Property
from CDUtilsPack import MiscUtils


# -------------------------------------------

_IsWin32 = True if (sys.platform == 'win32') else False

# -------------------------------------------

def _Build(batCmd : str, errorTextIfFail : str):
    
    def _ExecBat() -> bool:
        
        n = batCmd.find(' ')        
        batPath = batCmd[:n] if n >=0 else batCmd
        batArgs = batCmd[n:] if n >=0 else ""
        

        if not os.path.isabs(batPath):
            scpDir = CDUtils.GetScriptDir()
            batPath = os.path.join(scpDir, batPath)
            batPath = os.path.abspath(batPath)
            
        #print(batCmd, batPath)
        
        currDir = os.getcwd()  # store cwd
        
        os.chdir( os.path.split(batPath)[0] )
        
        res = os.system(batPath + batArgs)
        print()
        
        os.chdir(currDir)  # restore cwd
        
        return res == 0
    
    if not _ExecBat():
        print(errorTextIfFail)
        exit()

# -------------------------------------------

def _MakeArchive(archiveName : str, archiveNameExt : str, srcDir : str, cmdFormat : str):
    """
    'archiveName'    - name path or name, 
    'archiveNameExt' - archive file extension with leading dot,
    'srcDir'         - dir to pack,
    'cmdFormat'      - command in format "command {name}", where {name} is archive name, 
                       using to make archive.  
    """
    archiveName = InsertEnvVarsInPath(archiveName)

    split = os.path.split(srcDir) 

    if archiveName == "":
        archiveName = os.path.join( split[0], split[1] + archiveNameExt)
    
    extLen = len(archiveNameExt)
    if archiveName.upper()[-extLen:] != archiveNameExt.upper():
        archiveName = os.path.join( archiveName, split[1] + archiveNameExt )            
        
    if os.path.exists(archiveName): os.remove(archiveName)
                     
    print('Packing into ' + archiveName + '...')
    
    cmd = cmdFormat.format(name = archiveName)        
    print(cmd)
    
    res = os.system(cmd)
    if res != 0: 
        print('Archive error!')
        exit()
        
# -------------------------------------------
        
def _MakeZipArch(srcDir : str, dstDir : str):
         
    archName = os.path.split(srcDir)[1] + ".zip"
    fullArchName = InsertEnvVarsInPath( os.path.join(dstDir, archName) )
    
    print('Packing into ' + fullArchName + '...')
    
    MiscUtils.ZipCompress(srcDir, fullArchName)              
                  
# -------------------------------------------

def _FixShell(name : str) -> str:
    
    """ Fix shell file extension 
        add .bat or .sh to name OR
        change .bat -> .sh for Linux 
        + slash fix """
        
    def FixName() -> str:
    
        # name have .bat ext
        if name.upper()[-4:] == ".BAT":
            if not _IsWin32:
                return name[:-4] + ".sh"
        
        # name don't have ext
        if os.path.split(name)[1].find('.') < 0:
            return name + (".bat" if _IsWin32 else ".sh") 
        
        # else
        return name

    sl = ['/', '\\'] if _IsWin32 else ['\\', '/'] 
    return FixName().replace(sl[0], sl[1])

# -------------------------------------------

class Version:
    
    def __init__(self, publicVer : str):
        
        self.autoVer = self.__GetLastVer()
        self.publicVer = publicVer
        
    def FileName(self) -> str:
        
        name = "Ipsius_{0}_v{1}".format( CDUtils.DateTimeStamp( time.time() ), self.autoVer )
        if self.publicVer: name += '_' + self.publicVer
        return name
        
    def AsText(self) -> str:
        
        verTxt = "Auto version: {}".format(self.autoVer)
        if self.publicVer: verTxt += "; public version: {}".format(opts['pub_ver'])
        return verTxt
                
    @staticmethod
    def __GetLastVer() -> int: 
        
        """
            Find in listed paths file with max version,
            create pack named as: Ipsius_*_v*.  
        """
        
        CDir = '%IPSIUS_RELEASE%/IpsiusPacks'
        CNameFormat = 'Ipsius_*_v*'
        
        def findLastVersion(dir : str) -> int:                             

            def fileVer(f : str) -> int:
                name = os.path.split(f)[1] # remove path
                name = name.split('.')[0]  # remove extension       
                try:
                    strVal = name.split('_')[2] # = 'vXXXX'                    
                    return int(strVal[1:])                          
                except:
                    return 0                                                          

            dir = os.path.join(InsertEnvVarsInPath(CDir), dir)
            
            if not os.path.exists(dir) : 
                raise UserException(dir + ' not exist')
                
            searchPattern = os.path.join(dir, CNameFormat)
            filesList = glob.glob(searchPattern)
            
            if filesList: return max( [fileVer(f) for f in filesList] )
            return 0            

        return max( [findLastVersion(path) for path in _CPlatformsList] ) + 1

# ------------------------------------------- 

class _CmpVerInfo:
    
    def __init__(self, info : str, src : str or bytes):
        
        self.info = info
        self.error = None
        self.sign = None

        if isinstance(src, str):   # src is file name                            
            with open(src, mode='r+b') as f:
                data = f.read()                    
        else:                
            data = src
            
        def GetSign() -> str:                
            n = data.find(b'iCmp ver ')
            if n < 0: return None
            
            n2 = data.find(b'\x00', n + 1)
            if n2 < 0: return None
            
            return data[n:n2].decode('ascii')                
                            
        self.sign = GetSign()
        if not self.sign:
            self.error = 'Signature not found' 
            return
        
        # print(self.sign)  # debug
        
    Error = Property("error")
    
    def __OK(self):
        assert not self.error
        assert self.sign 
    
    def Equal(self, other) -> bool:

        self.__OK()
        other.__OK()
        
        return self.sign == other.sign
    
    def __str__(self):
        
        s = self.info + ':'
        
        if self.error:
            s += ' ERROR ' + self.error
        else:
            s += self.sign
        
        return s

# ------------------------------------------- 

class _Context:
    
    """ global context for store data """        
    
    def __init__(self, ver : Version):
        self.cmpVerInfo = []
        self.ver = ver
        
    def VerInfoAsText(self) -> str:
        return self.ver.AsText()
    
    def AddCmpVerInfo(self, info : _CmpVerInfo):
        self.cmpVerInfo.append(info)
                    
    def __CloseCmpInfo(self):
        
        try:
        
            # error in any info
            if any( [i.Error for i in self.cmpVerInfo] ):
                raise Exception('Error in item')
                            
            # just one item
            if len(self.cmpVerInfo) == 1:
                raise Exception('Single item!')
            
            # all items equal
            for indx, item in enumerate(self.cmpVerInfo, 1):
                if not item.Equal( self.cmpVerInfo[indx - 1] ):
                    raise Exception('Different sign!')
                
            # OK
            print('iCmp version: OK')
                    
        except Exception as e:
            
            # print info
            msg = '\n! WARNING ! iCmp version; ' + e.args[0] + '\n'
            for i in self.cmpVerInfo:
                msg += '    ' + str(i) + '\n'
            print(msg)
                    
    def Close(self):
        
        self.__CloseCmpInfo()


# -------------------------------------------

def _InitFwuItem(context : _Context, srcRoot : str) -> str:
            
    def GetFwuDir() -> str:
        
        releaseDir = os.getenv('IPSIUS_RELEASE')
        assert releaseDir
        
        return os.path.join(releaseDir, 'FWU/DevIpTdm/')
                    
    def FindLastFwu(dir) -> str:
        
        files = glob.glob( os.path.join(dir, '*.fwu') )
        files.sort()
        
        assert files
        
        return files.pop()
    
    dir = GetFwuDir()
    
    return FindLastFwu(dir)

# ------------------------------------------- 

def _InitInfoFile(context : _Context, srcRoot : str) -> str:

    svnRev = CDUtils.ExtractSvnRevision(srcRoot)
    assert svnRev

    info = 'Pack build: {0} by {1}; SVN: {2}'.format(time.ctime(), CDUtils.GetUserEnv(), svnRev)
    
    dst = os.path.join(srcRoot, 'Temp/deploy_build_info.txt')
    CDUtils.SaveStringList( [info,], dst )
    
    return dst

# ------------------------------------------- 

def _InitVersionFile(context : _Context, srcRoot : str) -> str:
    
    dst = os.path.join(srcRoot, 'Temp/ver.txt')
    CDUtils.SaveStringList( [context.VerInfoAsText(),], dst )
    
    return dst           

# ------------------------------------------- 
    
def _FinalizerCmpProtoInfo(context : _Context, srcFullName : str):
    
    def makeCmpInfoFromFWU(fwuName) -> _CmpVerInfo:                            
        zip = zipfile.ZipFile(fwuName, mode = 'r')                            
        try:
            return _CmpVerInfo( fwuName, zip.read('BfMainE1_packed.ldr') )
        finally:                
            zip.close()
            
    isFwu = (srcFullName.upper()[-4:] == ".FWU")            
    i = makeCmpInfoFromFWU(srcFullName) if isFwu else _CmpVerInfo(srcFullName, srcFullName)                      
    context.AddCmpVerInfo(i)
    
# ------------------------------------------- 

class Tags:
    """Common tags for all install descriptions."""
    def __init__(self, platformSpecific : [str] = []):
        self.tags = ['bin', 'bin_debug', 'doc', 'fwu', 'gui_config', 'redistr',  'redistr_debug']
        self.tags += platformSpecific
        
    def All(self) -> []:
        all = [] + self.tags
        all.remove('bin_debug')
        all.remove('arm')
        return all
    
    def VerifySet(self, tagsSet : [str]):  # can throw
        
        for tag in tagsSet:
            if tag not in self.tags:
                raise UserException('Unknown tag ' + tag)
            if (tag + '_debug') in tagsSet:
                raise UserException('_debug conflict for ' + tag)
                        
    def TagCorrect(self, tag):
        return tag in self.tags
        
    def Info(self):
        return ";".join(self.tags)

    @staticmethod
    def DebugTag(tagsSet : [str]) -> bool:
        return ('bin_debug' in tagsSet)
    
    @staticmethod
    def FixTagsForDebug(tagsSet : [str]) -> [str]:

        def Replace(t : str):
            if t not in lx: return
            lx.remove(t)
            lx.append(t + '_debug')
                            
        lx = list(tagsSet)
                
        if Tags.DebugTag(lx):
            Replace('redistr')
            Replace('pdb')
            
        return lx
       
# -------------------------------------------

_CPlatformsList = ['win32', 'linux', 'linux_arm']        

class _Platform:    
    
    def __init__(self, tags : Tags):
        
        arm = 'arm' in tags        
        
        self.list = _CPlatformsList
        
        if _IsWin32: 
            self.platform = self.list[0]
            assert not arm
        elif arm: 
            self.platform = self.list[2]
        else: 
            self.platform = self.list[1]
        
    def Is(self, platform : str) -> bool:
        assert platform in self.list
        return platform == self.platform
        
    def __str__(self): return self.platform
               
        
# -------------------------------------------


class Description:
        
    @staticmethod
    def CorrectTags() -> Tags:
        platformTags = ['pdb', 'pdb_debug'] if _IsWin32 else ['arm']
        return Tags(platformTags)    
    
    @staticmethod
    def GetInstallDesc(tags : [str], ver : Version) -> InstallDesc:
        
        correctTags = Description.CorrectTags()
        
        correctTags.VerifySet(tags)        
        platform = _Platform(tags)
                     
        i = InstallDesc(correctTags, _Context(ver) )        

        # platform related binaries         
        i.DefaultDestination = 'bin/'                
        if platform.Is('win32'):
            i.Add( 'ProjIpsius/Release/ProjIpsius.exe',       'bin',         info = True, finalizeFn = _FinalizerCmpProtoInfo )
            i.Add( 'ProjIpsius/Debug/ProjIpsius.exe',         'bin_debug',   info = True, finalizeFn = _FinalizerCmpProtoInfo )    
            i.Add( 'ProjIpsius/Release/vc80.pdb',             'pdb' )
            i.Add( 'ProjIpsius/Release/ProjIpsius.pdb',       'pdb' )
            i.Add( 'ProjIpsius/Debug/vc80.pdb',               'pdb_debug' )
            i.Add( 'ProjIpsius/Debug/ProjIpsius.pdb',         'pdb_debug' )            
            logDst = i.DefaultDestination + 'LogView.exe' 
            i.Add( 'ProjUdpLogViewer/Release/ULVWin32.exe',   'bin',       dst = logDst )
            i.Add( 'ProjUdpLogViewer/Debug/ULVWin32.exe',     'bin_debug', dst = logDst )            
        elif platform.Is('linux'):
            i.Add( 'ProjIpsius/bin/linux-gcc/release/ProjIpsius',   'bin',         info = True, finalizeFn = _FinalizerCmpProtoInfo )
            i.Add( 'ProjIpsius/bin/linux-gcc/debug/ProjIpsius',     'bin_debug',   info = True, finalizeFn = _FinalizerCmpProtoInfo )    
            logDst = i.DefaultDestination + 'LogView'
            i.Add( 'ProjUdpLogViewer/bin/linux-gcc/release/UdpLogViewer', 'bin',        dst = logDst )
            i.Add( 'ProjUdpLogViewer/bin/linux-gcc/debug/UdpLogViewer',   'bin_debug',  dst = logDst )
        elif platform.Is('linux_arm'):
            i.Add( 'ProjIpsius/bin/linux-buildroot/release/ProjIpsius',   'bin',         info = True, finalizeFn = _FinalizerCmpProtoInfo )
            i.Add( 'ProjIpsius/bin/linux-buildroot/debug/ProjIpsius',     'bin_debug',   info = True, finalizeFn = _FinalizerCmpProtoInfo )    
        else:
            assert None
            
        # platform related redistr
        i.DefaultDestination = 'bin/'                        
        if platform.Is('win32'):
            i.Add( '%QTDIR%/bin/QtCore4.dll',                  'redistr' )
            i.Add( '%QTDIR%/bin/QtGui4.dll',                   'redistr' )
            i.Add( '%QTDIR%/bin/QtMultimedia4.dll',            'redistr' )
            i.Add( '%QTDIR%/bin/QtNetwork4.dll',               'redistr' )
            i.Add( '%QTDIR%/plugins/imageformats/qjpeg4.dll',  'redistr',       dst = 'bin/imageformats/' )
            
            i.Add( '%QTDIR%/bin/QtCored4.dll',                 'redistr_debug' )
            i.Add( '%QTDIR%/bin/QtGuid4.dll',                  'redistr_debug' )
            i.Add( '%QTDIR%/bin/QtMultimediad4.dll',           'redistr_debug' )    			
            i.Add( '%QTDIR%/bin/QtNetworkd4.dll',              'redistr_debug' )
            i.Add( '%QTDIR%/plugins/imageformats/qjpegd4.dll', 'redistr_debug', dst = 'bin/imageformats/' )
                        
            i.Add( '%VS80COMNTOOLS%../../VC/redist/x86/Microsoft.VC80.CRT/',                      'redistr',       dst = 'bin/Microsoft.VC80.CRT/' )                        
            i.Add( '%VS80COMNTOOLS%../../VC/redist/Debug_NonRedist/x86/Microsoft.VC80.DebugCRT/', 'redistr_debug', dst = 'bin/Microsoft.VC80.DebugCRT/' )
        else:
            # Qt for linux/linux_arm
            i.Add( '%QTDIR%/lib/libQtCore.so.4',                 'redistr' )
            i.Add( '%QTDIR%/lib/libQtGui.so.4',                  'redistr' )
            i.Add( '%QTDIR%/lib/libQtNetwork.so.4',              'redistr' )
            i.Add( '%QTDIR%/lib/libQtMultimedia.so.4',           'redistr' )            
            i.Add( '%QTDIR%/plugins/imageformats/libqjpeg.so',   'redistr',       dst = 'bin/imageformats/' )
            i.Add( '%QTDIR%/lib/libQtCore.so.4',                 'redistr_debug' )
            i.Add( '%QTDIR%/lib/libQtGui.so.4',                  'redistr_debug' )
            i.Add( '%QTDIR%/lib/libQtNetwork.so.4',              'redistr_debug' ) 
            i.Add( '%QTDIR%/lib/libQtMultimedia.so.4',           'redistr_debug' )             
            #i.Add( '%QTDIR%/plugins/imageformats/libqjpeg.so',   'redistr_debug', dst = 'bin/imageformats/' )

        if platform.Is('linux_arm'):
            i.Add( '%QTDIR%/lib/fonts/',                         'redistr',       dst = 'bin/fonts/' )
            i.Add( '%QTDIR%/lib/fonts/',                         'redistr_debug', dst = 'bin/fonts/' )
            
        # lang
        i.DefaultDestination = 'bin/'                        
        i.Add( 'Src/iPult/tr/pult_ru.qm' )
        i.Add( 'Src/iPult/tr/pult_uk.qm' )
                                                         
        i.DefaultDestination = 'doc/'
        i.Add( 'Doc/DomainDRIManual.doc',        'doc' )
        i.Add( 'Doc/DriDss1ToSip.doc',           'doc' )
        #i.Add( 'Doc/Lucksi/Lucksi.doc',          'doc' )
                 
        i.DefaultDestination = '/'
        i.Add( _FixShell('PyIpsiusDeploy/files/run.bat') )
        i.Add( _FixShell('PyIpsiusDeploy/files/aoz.bat') )		
        i.Add( _FixShell('PyIpsiusDeploy/files/SetBoardNetwork.bat') )    
        i.Add( _FixShell('PyIpsiusDeploy/files/Pcid.bat') )    		
        i.Add( _FixShell('PyIpsiusDeploy/files/testSerial.bat') )    				
        i.Add( _FixShell('PyIpsiusDeploy/files/CallMeeting.bat') )    
        i.Add( _FixShell('PyIpsiusDeploy/files/CMRules.bat') )    		
        i.Add( _FixShell('PyIpsiusDeploy/files/logViewRun.bat') )
        i.Add( _FixShell('PyIpsiusDeploy/files/runPult.bat') )
        i.Add( _FixShell('PyIpsiusDeploy/files/setPath.bat') )
		
        if _IsWin32:
            # i.Add( '%IPSIUS_RELEASE%/Resdist/win32/msvc2005/vcredist_x86.exe', 'redistr' )
            i.Add( 'PyIpsiusDeploy/files/install.sfx' )
        
        i.Add( 'ProjIpsius/isc/Release/',                 dst = 'isc/' )
        i.Add( 'ProjIpsius/CallMeetingData/',             dst = 'CallMeetingData/' )
        
        if _Platform(tags).Is('linux_arm'): 
            i.Add( 'ProjIpsius/UT_MultiMediaTest/',       dst = 'UT_MultiMediaTest/' )
        
        i.Add( '', 'fwu', dst = 'firmware/', info = True, finalizeFn = _FinalizerCmpProtoInfo, initFn = _InitFwuItem)
        i.Add( '',        dst = 'build_info.txt',                         initFn = _InitInfoFile)
        i.Add( '',        dst = 'ver.txt',                                initFn = _InitVersionFile)
                
        # Python GUI config -- clean up .svn, .pyc
        i.Add( 'PyIpsiusQConfig/src/',                        'gui_config',  dst = 'PyIpsiusQConfig/src/' )
        i.Add( 'PyIpsiusQConfig/images/',                     'gui_config',  dst = 'PyIpsiusQConfig/images/' )        
        i.Add( 'PyLib/',                                      'gui_config',  dst = 'PyLib/' )
        i.Add( _FixShell('PyIpsiusDeploy/files/runGui.bat'),  'gui_config',  dst = '/' )
        
        if _IsWin32:
            i.Add( '%IPSIUS_RELEASE%/Resdist/win32/Python31/', 'gui_config',  dst = 'Python/' )
                        
        return i
        

    @staticmethod
    def BuildLog(tags : [str]):
        
        if _Platform(tags).Is('linux_arm'): 
            raise UserException("Can't build log for ARM!")
        
        CBuildRelease = r'..\..\ProjUdpLogViewer\_build_release_auto.bat'
        CBuildDebug   = r'..\..\ProjUdpLogViewer\_build_debug_auto.bat'
        _Build( _FixShell(CBuildDebug if Tags.DebugTag(tags) else CBuildRelease), 'Log build error!')
        
    @staticmethod
    def BuildIpsius(tags : [str]):
        CDebug   = { 'win32':      '..\..\ProjIpsius\_build_debug_auto.bat', 
                     'linux':      '../../ProjIpsius/_build_gcc.sh debug', 
                     'linux_arm':  '../../ProjIpsius/_build_arm.sh debug' }
                        
        CRelease = { 'win32':       '..\..\ProjIpsius\_build_release_auto.bat', 
                     'linux':       '../../ProjIpsius/_build_gcc.sh release', 
                     'linux_arm':   '../../ProjIpsius/_build_arm.sh release' }

        m = CDebug if Tags.DebugTag(tags) else CRelease
        _Build( m[ str(_Platform(tags)) ], 'Ipsius build error!' )
        
    @staticmethod
    def BuildLang(tags : [str]):
        
        CPath = '../../Src/iPult/'
        
        cmd =       CPath + '_tr_release_auto.bat' if _Platform(tags).Is('win32') \
              else  CPath + '_tr_release_auto.sh'
        _Build(cmd, 'Lang build error!')
                        
    @staticmethod
    def MakeArchive(dstDir : str, srcDir : str, tags : [str]):                              
        
        platform = _Platform(tags)
        
        if platform.Is('win32'):        
            commentName = os.path.join(srcDir, 'install.sfx')
            cmd = 'rar a -s -m1 -ep1 -sfx -r -inul -z"{comment}" {arch} "{src}\*.*"'.format(
                comment = commentName, arch = "{name}", src = srcDir)
            _MakeArchive(dstDir, ".exe", srcDir, '"' + cmd + '"')  
                      
        elif platform.Is('linux'):
            dirName = os.path.split(srcDir)[1]
            p = {'srcDir' : srcDir, 'name' : '{name}', 'dirName' : dirName}
            cmd = 'cd {srcDir} \ntar -czf "{name}" "../{dirName}"'.format(**p) # silent
            _MakeArchive(dstDir, ".tar.gz", srcDir, cmd)
            
        elif platform.Is('linux_arm'):
            _MakeZipArch(srcDir, dstDir)
            
        else: 
            assert None
                                
# -------------------------------------------

def DescriptionType():
    return Description
    
