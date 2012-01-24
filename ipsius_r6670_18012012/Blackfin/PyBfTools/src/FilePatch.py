'''
Created on 13.01.2010

@author: Alex
'''

import sys

import CDUtils


# --------------------------------------------------------------

# old and new values wrapper (implemented just for two bytes-strings)
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
                
        for i in self.pairs:
            processPair(i[0], i[1])
        
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
            return f.read()
    
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
    
    for f in files:
        log(f, False)
        data = loadFile(f)

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

"""

    Py -- file I/O free GIL?
    Async pattern -- do max to N async operations in background. Thread + Lock + 2x Condition (wake up read and writer). In: __iter__ return __call__ + max in queue. 
    Implement for files read; test read > process, read < process 
    Per file example -- CRC32 calc
    RegExp speed -- find() vs "abc", "abc|dcb", re.escape 
    Patch config -- use RegExp - ?
    
    Py auto property like Property("prop name") inside class; problem -- auto get class name. __name__ don't work, try thru stack frames 
    Prop = Property("prop") thru in-place exec(), return descriptor - ?
    Property just "mark", later transformed by decorator 


"""

# --------------------------------------------------------------

if CDUtils.IsMain():
    
    PatchMainCLI()

    pass

    
    