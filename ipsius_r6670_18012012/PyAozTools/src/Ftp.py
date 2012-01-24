'''
Created on 31.03.2011

@author: Valkiriy
'''

from ftplib import FTP, error_perm, error_reply, error_reply, error_proto
from CDUtilsPack.MetaUtils import UserException
from CDUtilsPack import MiscUtils
from CDUtilsPack.CoreTrace import CoreTrace
import zipfile
import os
import time

#import sys

def _GetLastFirmware(searchPattern) -> (int, str) or None:  #todo move to utils        
                        
        def ver(f : str) -> int or None:  #todo move to global for deploy
            name = os.path.split(f)[1] # remove path
            name = name.split('.')[0] # remove extension       
            try:
                strVal = name.split('_')[2] # = 'vXXXX'                    
                return int(strVal[1:])                          
            except:
                return None      
            
        return MiscUtils.FindLastFileRevision(searchPattern, ver)                                    


class _Const:
    IpsiusSDPath = 'ipsius'
    IpsiusSDPathForUpload = 'ipsius_new'
    
    
    VerFile = 'ver.txt'
    #Restart = 'Restart.sh'
    AutoVer = 'Auto version:'
    
    PackageNamePattern = '*_v*.zip'
    
    NetworkFile = 'network.cfg'
    
    TempFileDir = MiscUtils.GetScriptDir("../TmpDownload")
    
    ArchName = 'AozBackup.zip' # ts
    
    @staticmethod
    def ArchName():
        return '{}_AozBackup.zip'.format( MiscUtils.DateTimeStamp(time.time()) )    


class Ftp:        
    
    """
    Ftp client wrapper for Aoz service tasks
    """
    
    @staticmethod    
    def _Parse(info, key):
        try:
            rOfKey = info.split(key)[1]                
            return int(rOfKey.split()[0])
        except (IndexError, ValueError):
            return None
    
    def __init__(self, tracer : CoreTrace, ip : str, port : int, login, pwd = ''):  
              
        self.ftp = FTP()                                    
        
        self.Trace = lambda msg: tracer.Add('Ftp', msg) 
        self.Trace( 'connecting to {0}:{1} as {2}...'.format(ip, port, login) )
        
        try:
            self.ftp.connect(ip, port)
            self.ftp.login(login, pwd)
        except (error_perm, error_reply, error_reply, error_proto) as e:
            UserException(e)         
    
        self.Trace('connected')
        
        
    def CurrPackageVer(self) -> int or None:            
        
        def getInfo():
            verInfo = []                                    
            
            try:              
                fileOnSD = _Const.IpsiusSDPath + '/' +  _Const.VerFile            
                self.ftp.retrlines('RETR ' + fileOnSD, lambda line: verInfo.append(line))
                return verInfo
            except error_perm as e:
                self.Trace('can`t define previous version ({})'.format(e))                           
                return None                
        
        # -------------------------------    
        
        info = getInfo()        
        if info == None: return None
        
        for line in info:
            ver = self._Parse(line, _Const.AutoVer)
            if ver != None: return ver
            
        return None
    
    # -------------------------------

    def _Dir(self) -> [(bool, str)]: # (IsDir, Name)
        resp = []
        self.ftp.retrlines('LIST', lambda line: resp.append(line))
        
        result = []
        for line in resp:
            sl = line.split()# -rw-r--r-- 1 owner groupe size Mm dd hh:mm file            
            result.append( (sl[0][0] == 'd', sl[8]) )
        return result     
    
    # -------------------------------
    
    def _ListDirFile(self, path, topdown, callback): # path in curr dir                
        
        currParh = ''
        
        def cwd(path): #return parh
            self.ftp.cwd(path)
            nonlocal currParh
            currParh += '\\' + path
            
        def up():            
            nonlocal currParh
            self.ftp.cwd('..')            
            currParh = currParh.rsplit('\\', 1)[0]                                
        
        def Yield(name, isDir):
            #yield (currParh, name, isDir)
            callback(currParh, name, isDir)               
        
        def walk(path):
            if not topdown : Yield(path, True)
            if not path == '': cwd(path)                     
            
            fileList = self._Dir()        
                    
            for curr in fileList:
                if curr[0] == False:    Yield(curr[1], False) # is file
                else:                   walk(curr[1])                
                            
            up()
            if topdown : Yield(path, True)                                      
           
        walk(path)     
             
        
    def _DelRecursive(self, path): # path in curr dir
          
        def rmItem(currParh, name, isDir):
            
            if isDir:
                self.ftp.rmd(name)
            else:
                self.ftp.delete(name)
                             
        self.Trace('rmPath ' + path)        
        self._ListDirFile(path, topdown = True, callback = rmItem)
        
    #----------------------------------------------------------------------    
            
    def Upload(self, src : str, forced): 
        
        def currPathExist(name, isDir):
            infoList = self._Dir()            
            return (isDir, name) in infoList                                                              
        
        def getVerFromArch(file):    
            zip = zipfile.ZipFile(file, mode = 'r')
            binVer = zip.read(_Const.VerFile)                        
            return self._Parse(binVer.decode(), _Const.AutoVer)
        
        def selectSrc():            
            """ 
            Select source file. 
            Return None if appropriate file not exist, can raise UserException. 
            """                                            
            if os.path.isfile(src) and forced:
                return src
            elif os.path.isfile(src):                                                                                        
                archVer = getVerFromArch(src)
                if archVer == None: raise UserException('Source has no ' + _Const.VerFile)    
                currVer = self.CurrPackageVer()
                if currVer == None or currVer >= archVer: return None
                return src            
            else: # src is path                
                finded = _GetLastFirmware(os.path.join(src, _Const.PackageNamePattern))                
                if finded == None:
                    raise UserException(
                    '{0} not fond in path {1}'.format(_Const.PackageNamePattern, src)
                    )                
                assert(getVerFromArch(finded[1]) == finded[0])
                if forced: return finded[1]
                currVer = self.CurrPackageVer()
                if currVer == None: return finded[1]                 
                if finded[0] > currVer: return finded[1]
                return None 
             
        def upload(srcFile):            
            zip = zipfile.ZipFile(srcFile, mode = 'r')
            zipFiles = zip.namelist() 
            
            createdDir = [_Const.IpsiusSDPathForUpload + '/', ] 
            for fileName in zipFiles:
                zipFile = zip.open(fileName)
                fileName = fileName.replace("\\", "/")
            
                sdPath = _Const.IpsiusSDPathForUpload + '/' + os.path.split(fileName)[0]            
            
                if (sdPath not in createdDir):
                    self.ftp.mkd(sdPath)
                    createdDir.append(sdPath)            
                    
                fileNameOnSd = _Const.IpsiusSDPathForUpload + '/' + fileName    
                
                if not fileNameOnSd[-1:] == '/': 
                    self.Trace('Send ' + fileNameOnSd + '...')
                    self.ftp.storbinary('STOR ' + fileNameOnSd, zipFile)                      
            
        # process Upload
        self.Trace('upload from {0}, forced = {1}'.format(src, forced))
        
        srcFile = selectSrc()        
        if srcFile == None: return                        
        
        if currPathExist(_Const.IpsiusSDPathForUpload, True):                    
            self._DelRecursive(_Const.IpsiusSDPathForUpload)            
            
        self.ftp.mkd(_Const.IpsiusSDPathForUpload)       
 
        self.Trace('start upload ' + srcFile) 
        upload(srcFile)      
        
    #----------------------------------------------------------------------
        
    def SetNetwork(self, ip, mask, gateway, dns):
        
        class FileLike:
            def __init__(self):
                self.lines = []
                self.iter = 0
            
            def readline(self):
                if len(self.lines) <= self.iter: return '' #EOF
                result = self.lines[self.iter]
                self.iter += 1
                return result
                
            def Add(self, msg : str):
                self.lines.append(msg.encode())                        
        
        tmpFile = FileLike()
        tmpFile.Add('IP=' + ip)
        tmpFile.Add('Mask=' + mask)
        tmpFile.Add('Gateway=' + gateway)
        tmpFile.Add('DNS=' + dns)                                              
         
        self.ftp.storlines('STOR ' + _Const.NetworkFile, tmpFile)         
        
    #----------------------------------------------------------------------
        
    def Download(self, dst):                                      
        
        def saveFile(localNme, ftpName):
            #print('Save ', ftpName)
            f = open(localNme, 'wb')
            self.ftp.retrbinary('RETR ' + ftpName, f.write)            
        
        def addItem(currParh, name, isDir):                       
            fullLocalName = _Const.TempFileDir + currParh + '\\' + name            
            if isDir and not name == '':                
                os.makedirs(fullLocalName)
            elif not isDir:
                self.Trace('recv ' + currParh + '\\' + name + '...')
                saveFile(fullLocalName, name)                                       
                
        # del if Exist                             
        if os.path.exists(_Const.TempFileDir):  
            MiscUtils.DeleteFolderRecursive(_Const.TempFileDir)                     
                                    
        os.makedirs(_Const.TempFileDir)
         
        self.Trace('start download SD card content:')       
        self._ListDirFile('', topdown = False, callback = addItem)        
        
        if os.path.isdir(dst):
            if not os.path.exists(dst): os.makedirs(dst)             
            dst = os.path.join(dst, _Const.ArchName())
    
        self.Trace('download complete.')
                      
        self.Trace('package into ' + dst + '...')
        
        MiscUtils.ZipCompress(_Const.TempFileDir, dst)
                
        self.Trace('archive created.')
        
        MiscUtils.DeleteFolderRecursive(_Const.TempFileDir) 
        
    
    
     
        
        
        
        
   
        
                                
                                                     
                
                
           
            
        
      
         

      
        
        
