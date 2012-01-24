
import io
import os.path
import time
import datetime
import pickle
import urllib.request
import codecs
import os
import sys
import unittest
import inspect
import threading
import imp
import zipfile
import glob

from types import ModuleType
from collections import Iterable

from CDUtilsPack.StrUtils import LoadStringList

from TestDecor import UtTest

# -----------------------------------------------------     
 
def Foreach(iter, fn, *arg):
    for i in iter: fn(i, *arg)

    
def FindFirst(iter, fn, returnItem = True, *arg):
        
    for indx, i in enumerate(iter): 
        if fn(i, *arg):
            if returnItem: return i
            else: return indx
        
    return None


def TestBit(val, bitNum : int) -> bool:
    return (val & (1 << bitNum)) != 0 


def SetBit(val, bitNum : int) -> int:
    return (val | (1 << bitNum))


class MiscTinyTests(unittest.TestCase):
    
    def test_Foreach(self):
        
        lx = [1, 2, 3]
        res = []
        
        Foreach(lx, lambda i, add: res.append(i + add), 10)
        
        self.assertEqual(res, [11, 12, 13])
        
    def test_FindFirst(self):
        
        lx = [1, 2, 3]
        
        self.assertEqual( FindFirst(lx, lambda i: i == 2), 2)
        self.assertEqual( FindFirst(lx, lambda i, val: i == val, False, 3), 2)
        
    def test_Bits(self):
        self.assertEqual( TestBit(0x10, 4), True)
        self.assertEqual( TestBit(0x10, 5), False)
        self.assertEqual( SetBit(0x10, 0), 0x11)
        
# -----------------------------------------------------     

class QuickFileCache:
    
    def __init__(self, fileName):
        if not os.path.exists(fileName): raise Exception()
        self.name = fileName
        self.size = os.path.getsize(fileName)
        self.time = os.path.getmtime(fileName)
        
        self.captureTime = time.time()  # service 
        
    def Equal(self, other) -> bool:
        return (self.name == other.name and 
                self.size == other.size and 
                self.time == other.time)
    
    @property    
    def TimeAsString(self):
        return time.ctime(self.time)

    @staticmethod
    def Save(fileName, cacheFileName : str):
        info = QuickFileCache(fileName)
        with io.open(cacheFileName, 'w+b') as f:
            f.write( pickle.dumps(info) )

    @staticmethod
    def VerifyCache(cacheFileName : str) -> bool:
        if not os.path.exists(cacheFileName): return False
                
        with io.open(cacheFileName, 'r+b') as f:
            data = f.read()
            info = pickle.loads(data)
            
        if not os.path.exists(info.name): return False
        return info.Equal( QuickFileCache(info.name) )

@UtTest
def Test_QuickFileCache():
    pass

# -----------------------------------------------------     
        
class StatElement:
    def __init__(self):
        self.sum = 0
        self.count = 0
        
    def Add(self, i):
        self.sum += i
        self.count += 1
        
        if self.count == 1:
            self.min = i
            self.max = i
        else:
            if i < self.min: self.min = i
            if i > self.max: self.max = i
            
    @property
    def Sum(self): return self.sum
    
    @property 
    def Min(self): return self.min
    
    @property 
    def Max(self): return self.max
    
    @property 
    def Avg(self): return self.sum / self.count
                                             
    def __str__(self):
        if self.count == 0: return ""
        return "min {0} max {1} avg {2} count {3}".format(
        self.min, 
        self.max, 
        self.Avg, 
        self.count)

@UtTest
def Test_StatElement(verbose = False):
    se = StatElement()
    se.Add(1)
    se.Add(2)
    se.Add(3)
        
    assert se.Sum == 6
    assert se.Min == 1
    assert se.Max == 3
    assert se.Avg == 2

    s = str(se)
    
    if verbose: print(s)

# --------------------------------------------------
    
def LoadWebPage(url : str, data : str = None, headers : ((str, str),) = None) -> str:
    
    def GetCharsetFromHeaderInfo(info):    
        CCharSet = 'charset=' 
        
        content = info['Content-Type']    
        indx = content.find(CCharSet)
        assert (indx >= 0) 
        
        return content[indx + len(CCharSet):]
        
    req = urllib.request.Request(url)
    
    req.add_header('Accept-Charset', 'utf-8')
    
    if headers is not None:
        for h, hVal in headers:
            req.add_header(h, hVal)

    f = urllib.request.urlopen(req, data)    
    page = f.read()
            
    charset = GetCharsetFromHeaderInfo( f.info() )
    codec = codecs.lookup(charset)
    
    return codec.decode(page)[0]

@UtTest
def Test_LoadWebPage():
    pass

# --------------------------------------------------

def GetScriptDir(addRelativePath = "") -> str:
    dir = os.path.split(sys.argv[0])[0]    
    dir = os.path.abspath(dir)       
    
    if addRelativePath:
        dir = os.path.abspath( os.path.join(dir, addRelativePath) )
          
    return dir

# --------------------------------------------------

def DirUp(dir : str, level : int) -> str:
    if level == 0: return dir
    indexes = [i for i, c in enumerate(dir) if c == os.sep]
    assert len(indexes) >= level
    return dir[:indexes[-level]]

# -----------------------------------------------------     

def IsMain() -> bool:
    
    frame = inspect.currentframe().f_back
    
    try:        
        return frame.f_globals['__name__'] == '__main__' 
    finally:
        del frame
        
# -----------------------------------------------------     

if sys.platform == 'win32':

    import msvcrt 

    # non blocking, return key code or None
    def KeyWasPressed() -> int or None:
        if msvcrt.kbhit(): return ord(msvcrt.getch()) 
        return None
            
else:    
    pass    

# -----------------------------------------------------     

class IterInThread:
    
    """ Enumerate seq in background thread """
    
    def __init__(self, seq : Iterable, maxInQueue : int):
        
        def ThreadBody():
            
            def Add(last : bool, result):
                with self.lock:
                    self.queue.append( (last, result) )
                    self.wakeupIter.notify_all()
                            
            for i in seq:                                
                # push result
                Add(False, i)
                
                # can process next item?
                with self.lock:
                    if len(self.queue) >= maxInQueue:
                        self.wakeupThread.wait()
                                    
            # add 'last' item
            Add(True, None)
                        
        self.lock = threading.Lock()
        self.wakeupThread = threading.Condition(self.lock)
        self.wakeupIter = threading.Condition(self.lock)
        
        self.queue = []  # array of (last : bool, result)
        
        self.thread = threading.Thread(target = ThreadBody)
        self.thread.start()
        
    def __iter__(self):
        
        while True:            
            with self.lock:                
                if len(self.queue) == 0:
                    self.wakeupThread.notify_all()                    
                    self.wakeupIter.wait()
                else:
                    result = self.queue[0]
                    self.queue = self.queue[1:]                    
                    if (result[0]): break
                    self.wakeupThread.notify_all()
                    yield result[1]                    
                
# -----------------------------------------------------

def CallInfo() -> str:
            
    f = inspect.currentframe().f_back

    try:        
        name = f.f_code.co_name        
        v = inspect.getargvalues(f)
        
        res = name + '('

        args = [arg + ' = ' + repr(v.locals[arg]) for arg in v.args]
        res += (", ").join(args) + ')'
                                
    finally:
        del f
        
    return res

# -----------------------------------------------------

def LoadModuleFromFile(name : str) -> ModuleType:
    
    assert os.path.isabs(name)
    path, fileName = os.path.split(name)
    
    n = fileName.find('.')    
    assert n >= 0
    assert fileName[n+1:].upper() == "PY"    
        
    moduleName = fileName[:n]
    fp, pathname, description = imp.find_module(moduleName, [path])
        
    with fp:
        return imp.load_module(moduleName, fp, pathname, description)

@UtTest    
def Test_LoadModuleFromFile(silence : bool = True):
    
    m = LoadModuleFromFile(sys.argv[0])
    m = LoadModuleFromFile( GetScriptDir("./StrUtils.py") )
    
    if not silence: print(m, dir(m))   

# -----------------------------------------------------

def ExtractSvnRevision(svnRootPath : str) -> int or None:
    
    name = os.path.join(svnRootPath, '.svn/entries')
    if not os.path.exists(name): return None

    svnEntr = LoadStringList(name)
        
    for indx, s in enumerate(svnEntr):
        if s == "dir": return int( svnEntr[indx + 1] ) 
    
    return None

# -----------------------------------------------------

""" Convert current time to (yy)yymmdd(hhmm) """
def DateTimeStamp(timeSec : float, addTime = True, longYear = False) -> str:
        
    yStr = '%Y' if longYear else '%y'
    timeStr = '%H%M' if addTime else ''
    s = yStr + '%m%d' + timeStr
    
    return time.strftime(s, time.localtime(timeSec)) 

# -----------------------------------------------------

def AssertForException(f, exception = Exception):
    assert isinstance(exception, type)
    ok = False
    try:
        f()
    except Exception as e:
        ok = isinstance(e, exception)
    assert ok

# -----------------------------------------------------

def FileDaysOld(fileName : str) -> int:    
    fileTime = os.path.getmtime(fileName)
    delta = datetime.timedelta(seconds = time.time() - fileTime)
    return delta.days

# -----------------------------------------------------

def FileDatetime(fileName : str) -> str:
    fileTime = os.path.getmtime(fileName)
    return str( time.ctime(fileTime) )

# -----------------------------------------------------
    
def GetUserEnv():
    CName = 'USERNAME' if sys.platform == 'win32' else 'USER'
    return os.getenv(CName, '<undefined>')
    
# -----------------------------------------------------

def _FileTest():
    CName = r"d:\proj\Ipsius\.hgignore"
    print(FileDatetime(CName), 'age', FileDaysOld(CName))

# -----------------------------------------------------

def ZipCompress(src : str, dst : str):
        
    zip = zipfile.ZipFile(dst, mode = 'w')
    
    for path, dir, files in os.walk(src):             
        for file in files:                 
            pathInArch = path.replace(src, '')                
            zip.write(os.path.join(path, file), os.path.join(pathInArch, file))
            
    zip.close()
    
# -----------------------------------------------------
    
def DeleteFolderRecursive(topDir):    
    
    for path, dirs, files in os.walk(topDir, topdown = False):                
        for file in files: os.remove( os.path.join(path,file) )
        for dir in dirs: os.rmdir( os.path.join(path,dir) )
    
    os.rmdir(topDir)
        
# -----------------------------------------------------

def FindLastFileRevision(searchPattern : str, extractRevision) -> (int, str) or None:

    """    
    extractRevision - Fn(fileName : str) -> int
    Return (file revision, file name) 
    """       
                
    filesList = glob.glob(searchPattern)
    
    # only correct names, list of (rev, name)    
    files = []
    for f in filesList:
        rev = extractRevision(f)
        if rev is None: continue
        files.append( (rev, f) )
    
    if len(files):
        return max(files, key = lambda f: f[0])
    
    return None

# -----------------------------------------------------
     
if IsMain():
    
    _FileTest()
    
    unittest.main()
    
    pass
    
