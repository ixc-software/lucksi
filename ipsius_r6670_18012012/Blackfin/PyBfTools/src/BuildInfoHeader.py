'''
Created on 04.02.2010

@author: Alex
'''

import sys
import os.path
import os
import time
import io
import binascii

import CDUtils

# -----------------------------------------------------------------

def GetCrc32ForFile(fileName : str) -> int:
    
    CBlockSize = 64 * 1024
    crc = 0 # 0xffffffff
    size = 0
    
    with io.open(fileName, mode = "rb") as f:
        
        while True:
            b = f.read(CBlockSize)
            if b: 
                crc = binascii.crc32(b, crc)
                size += len(b)
            if not b or len(b) != CBlockSize: break
                        
    return crc & 0xffffffff

# -----------------------------------------------------------------

def SlFind(sl : [str], tag : str) -> int:
    indx = [indx for indx, s in enumerate(sl) if s.find(tag) >= 0] 
    assert len(indx) == 1
    return indx[0]

# -----------------------------------------------------------------

_CTemplate = """

#ifndef __BUILDINFO__
#define __BUILDINFO__

    // Don't edit this file!
    
    #ifdef NDEBUG 
        #define DEBUG_INFO ";RELEASE"
    #else
        #define DEBUG_INFO ";DEBUG"
    #endif
    
    #define ADD_DEBUG(msg) msg DEBUG_INFO
        

    struct AutoBuildInfo
    {
        static const char* FullInfo()
        {
            return "";   // FULL_INFO 
        }
        
        static const char* CmpProtoInfo()
        {
            return "";   // CMP_INFO
        }
        
        static int Revision()
        {
            return 0;    // REVISION
        }
    };
    
    #undef DEBUG_INFO
    #undef ADD_DEBUG

#endif

"""

def _StoreHeaderTemplate(fileName):
    dir = CDUtils.GetScriptDir()
    name = os.path.join(dir, fileName)
    CDUtils.SaveStringList( [_CTemplate,], name )

# -----------------------------------------------------------------

def _GetCmpInfo(cmpSrcPath : str) -> str:
    
    if not cmpSrcPath: return ""
    
    protoFile     = os.path.join(cmpSrcPath, "ChMngProto.h")
    protoDataFile = os.path.join(cmpSrcPath, "ChMngProtoData.h")
    
    protoFileCrc     = GetCrc32ForFile(protoFile)
    protoDataFileCrc = GetCrc32ForFile(protoDataFile)
    
    def GetVersion() -> int:
        sl = CDUtils.LoadStringList(protoFile)
        i = SlFind(sl, "CCmpProtoVersion =")
        s = sl[i]
        n = s.find("=")
        s = s[n+1:].strip()
        n = s.find(" ")
        return int(s[:n])
        
            
    cmpVer = GetVersion()
    
    h = CDUtils.IntToHex
    
    return "iCmp ver {0} proto-crc {1} data-crc {2}".format(cmpVer, h(protoFileCrc), h(protoDataFileCrc))

# -----------------------------------------------------------------

def GetSvnRev(path : str) -> int:
    import subprocess
    
    def findNumEnd(s : str) -> int:
        
        for i, curr in enumerate(s):
            if not curr.isdigit(): return i
                
        assert False
    
    s = subprocess.check_output(["svn", "info"], stderr = subprocess.STDOUT, cwd = path).decode('cp866')

    CRev = 'Revision: '
    n = s.find(CRev)
    assert n >= 0
    
    svn = s[n + len(CRev):]
    nEnd = findNumEnd(svn)
           
    return int(svn[:nEnd])
    

# -----------------------------------------------------------------

def UpdateHeader(name, svnPath, cmpSrcPath : str):
                        
    def getSvnRevision() -> int:
        name = os.path.join(svnPath, "entries")
        svnEntr = CDUtils.LoadStringList(name)
        
        for indx, s in enumerate(svnEntr):
            if s == "dir": return int(svnEntr[indx + 1]) 
        
        assert False, "Can't get SVN revision"
        
    def PatchValue(sl : [str], tag, value, addQuote = True):
        indx = SlFind(sl, tag)
        s = sl[indx]
        n1 = s.find("return ")
        n2 = s.rfind(";")
        assert n1 >= 0 and n2 >= 0
        
        if isinstance(value, str) and addQuote: value = '"' + value + '"'
        s = s[:n1+7] + str(value) + s[n2:]
        sl[indx] = s 
        
    if not os.path.isabs(name): name = os.path.abspath(name)
        
    if not os.path.exists(name):
        _StoreHeaderTemplate(name)
            
    sl = CDUtils.LoadStringList(name)
    
    svnRev = GetSvnRev( os.path.split(name)[0] ) # getSvnRevision()
    
    dateStr = time.ctime()    
    user = CDUtils.GetUserEnv()
    cmpInfo = _GetCmpInfo(cmpSrcPath)
    
    fullInfo = "AutoBuildInfo: rev {0}; build {1} by {2}".format(svnRev, dateStr, user)
    # if cmpInfo: fullInfo += "; " + cmpInfo
    
    PatchValue( sl, 'FULL_INFO', 'ADD_DEBUG("{msg}")'.format(msg = fullInfo), addQuote = False )
    PatchValue( sl, 'REVISION', svnRev )
    PatchValue( sl, 'CMP_INFO', cmpInfo )
    
#    print(*sl, sep='\n')

    CDUtils.SaveStringList(sl, name)

# -----------------------------------------------------------------

def Main(args : [str]):
      
    if len(args) not in (2, 3):
        print("Usage: script <header file name> <svn root path -- obsolete> [path to iCmp]")
        return

    cmpSrcPath = "" if len(args) < 3 else args[2]
              
    UpdateHeader(args[0], args[1], cmpSrcPath)

# -----------------------------------------------------------------

def CrcTest():
    
    CFile = r"d:\temp2\null.txt" 
    CFile = r"d:\temp2\rar.txt"
         
    crc = GetCrc32ForFile(CFile)
    print( CDUtils.IntToHex(crc) )
    
    exit()
    
def CmpTest():
    
    s = _GetCmpInfo(r"..\..\..\Src\iCmp")
    print(s)
    
    exit()
    
def RunTest():
    
    lx = ['a0.h', r'..\..\..\.svn']    
    Main(lx)
    
    lx[0] = 'a1.h'
    lx.append(r"..\..\..\Src\iCmp")    
    Main(lx)
    
    exit()     

# -----------------------------------------------------------------

if __name__ == '__main__':
    
#    CrcTest()
#    CmpTest()
#    RunTest()

#    print(GetSvnRev('s:/proj/Ipsius/Blackfin/PyBfTools/src/'))
#    exit()
        
    Main(sys.argv[1:])
    
    pass




