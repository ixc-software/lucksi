'''
Created on 13.01.2010

@author: Alex
'''

import sys
import threading
import time
from collections import Iterable
import binascii
import io

import CDUtils

from CDUtils import IterInThread as BackgroundEnumerator

# --------------------------------------------------------------

# old -> new values patch 
class FilePatcher:
    
    # pairs is tuple of (oldValue, newValue)
    def __init__( self, pairs : ((bytes, bytes),) ):
        
        self.pairs = []

        def Add(old, new):        
            assert isinstance(old, bytes) and len(old) > 0
            assert isinstance(new, bytes) and len(old) == len(new)
            
            self.pairs.append( (old, new) )

        for i in pairs:
            Add(i[0], i[1])
        
    # return the patch list (offset, bytesToPatch)
    def Process(self, data : bytes) -> [(int, bytes)]:
        res = []
                        
        def processPair(old, new):
            currOffs = 0        
            while True:
                pos = data.find(old, currOffs)
                if pos < 0: break
                res.append( (pos, new) )
                currOffs = pos + len(old)

        # do pairs
        for i in self.pairs:
            processPair(i[0], i[1])
            time.sleep(0)  # for background load thread 
        
        return res
                              

# --------------------------------------------------------------

def FilePatch(filesWildcard : (str), values : FilePatcher, 
              backup : bool = False, silenceMode : bool = False):
    import glob, io, sys, shutil, time
    
    def log(s, lf : bool = True):
        if silenceMode: return
        printEnd = '\n' if lf else '' 
        print(s, end = printEnd)
        sys.stdout.flush()
        
    def loadFile(fileName) -> bytes:
        with io.open(fileName, 'rb') as f:
            fileSize = f.seek(0, 2)
            f.seek(0)
            b = bytearray(fileSize)
            assert f.readinto(b) == fileSize
            return b # f.read(fileSize)
        
    def FilesEnum(files : [str]) -> Iterable:
        
        def fn():        
            for f in files:
                yield (f, loadFile(f))
                
        return BackgroundEnumerator(fn(), 3)
                
    files = []
    for w in filesWildcard:
        files.extend( glob.glob(w) )
    if len(files) == 0: 
        log("Nothing to process!")
        return
    
    benchBytes = 0
    benchTimeSec = 0.0

    # files loop
    filesPatched = 0
    tStart = time.clock()
    
    for f, data in FilesEnum(files):
        log(f, False)
#        data = loadFile(f)

        t = time.clock()
        
        patches = values.Process(data) # list of (offs, value)
                        
        benchTimeSec += (time.clock() - t)
        benchBytes += len(data)
               
        if len(patches) > 0:
            filesPatched += 1
            log(" " + str(len(patches)), False)
            
            if backup:
                log(" BACKUP", False) 
                shutil.copy(f, f + ".bak")

            with io.open(f, 'r+b') as f:   # apply patches
                for p in patches:
                    f.seek(p[0])
                    f.write(p[1])
                    
        if CDUtils.KeyWasPressed() == 27:  # ESC
            log("")
            log("User break!")
            break
            
        log("", True)   # file processing done
                
    tTotal = time.clock() - tStart

    # stats
    dataMb = benchBytes / (1024*1024)
    rateTotal = dataMb / tTotal    
    rateSearch = dataMb / benchTimeSec
    log( "Done, data {0:.0f} Mb, patched {1}, total rate {2:.1f} Mb/sec, search rate {3:.1f} Mb/sec".format(
            dataMb, filesPatched, rateTotal, rateSearch) )
            

# --------------------------------------------------------------

def ArrayBench(count, fn):
    
    import time
    
    print("Started")
    
    data = b"1" * count
    
    sum = 0
    
    t = time.clock()    
    for b in data:
        # fn(b)      # call
        sum += b     # math
    t = time.clock() - t
    
    print( "Rate {0:.1f} Mb/s".format(count / (1024*1024) / t) )
        

# --------------------------------------------------------------

def PatchMainCLI():
    CUsage = "FilePatch.py <patch> <wildcard> [<wildcard>] [-no_backup]"
    
    if len(sys.argv) < 3:
        print(CUsage)
        exit()
    
    # make patch
    patchSrc = "\n".join(CDUtils.LoadStringList(sys.argv[1]))
    patch = eval(patchSrc)
    
    # make wildcards
    backup = True
    files = []
    
    for s in sys.argv[2:]:
        if s.startswith("-"):
            if s == "-no_backup":
                backup = False
                continue
            raise Exception("Unknown options {0}".format(s))
        
        files.append(s)
    
    # run    
    FilePatch(files, FilePatcher(patch), backup)

# --------------------------------------------------------------

# return (crc32, rate)
def CalcCRC32(fileName : str) -> (int, float):        
    t = time.clock()
    
    with io.open(fileName, 'rb') as f:
        data = f.read()

    sizeMb = len(data) / (1024 * 1024)
    crc = binascii.crc32(data)
    
    t = time.clock() - t

    return (crc, sizeMb / t)

def CalcCRC32Threaded(fileName : str) -> (int, float):

    CLastDw = 0xffffffff
    CBlockSize = 512 * 1024

    def LoadNext():
        with io.open(fileName, 'rb') as f:
            while True:
                data = f.read(CBlockSize)
                if len(data) > 0: yield data
                if len(data) != CBlockSize: break
                                
    t = time.clock()

    size = 0
    crc = 0
    
    for block in BackgroundEnumerator(LoadNext(), 2):
        crc = binascii.crc32(block, crc)
        size += len(block)
        
    sizeMb = size / (1024 * 1024)
        
    crc = crc & CLastDw
    
    t = time.clock() - t

    return (crc, sizeMb / t)

    
# --------------------------------------------------------------

"""

    RegExp speed -- find() vs "abc", "abc|dcb", re.escape 
    Patch config -- use RegExp - ?

    Property:    
    Py auto property like Property("prop name") inside class; 
    problem -- auto get class name. __name__ don't work, try thru stack frames 
    Prop = Property("prop") thru in-place exec(), return descriptor - ?
    Property just "mark", later transformed by decorator 
    CDUtils -> package (common for EnumClass, Property, IsMain)


    1) per files (patch) -- no speed up (code example for BackgroundEnumerator - ?)
    2) per file (CRC) -- 10-20%  (code example)
    3) yes, I/O don't block GIL. bad GIL switch (use sleep(0)), no priority -- Learn.ThreadTest()
    4) BackgroundEnumerator pattern
    
"""

# --------------------------------------------------------------

def TestRun():
    
    CFiles = r"c:\Dev\boost_1.36.0\bin.v2\libs\*math*.lib"
    
    CReplacePair = (b"-=2020=-", b"-=1960=-")
    values = []
    for i in range(16): values.append(CReplacePair)
    
    FilePatch((CFiles, ), FilePatcher(values), True)
    
    pass

def TestCRC32():
    CFile = r"d:\_Torrent\06\boxing\Boxing - Andrew Golota Vs Riddick Bowe I (11-07-1996).avi"
#    CFile = r"d:\_Torrent\06\boxing\boxeo Lennox Lewis vs Oliver McCall II.avi"

    print("Started!")
    print(CalcCRC32Threaded(CFile))              
    print(CalcCRC32(CFile))

# --------------------------------------------------------------

if CDUtils.IsMain():
    
#    PatchMainCLI()

#    TestRun()
    TestCRC32()
    