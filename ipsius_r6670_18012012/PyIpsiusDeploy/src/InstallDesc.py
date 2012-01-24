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

import CDUtils
from CDUtils import UserException
from CDUtils import Property

# -------------------------------------------

def CopyFile(src, dst : str):
#            print(src, '->', dst)
                
    # create dst dir
    dst = dst.replace('\\', '/')
    
    dstPath = dst
    if not dstPath.endswith('/'):
        dstPath = os.path.split(dstPath)[0]

    if not os.path.exists(dstPath): 
        os.makedirs(dstPath)
        
    # copy
    shutil.copy(src, dst)
    
# -------------------------------------------

def PathTreeScan(path : str, callback):
    """ callback is f(fullPath : str, subPath : str, isDir : bool) -> bool  """
    
    join = os.path.join
    
    def Scan(basePath, subPath):

        fullPath = join(basePath, subPath)
                
        lx = os.listdir(fullPath)
        for f in lx:
            currF = join(fullPath, f)
            isDir = os.path.isdir(currF)
            currSubPath = join(subPath, f)
            result = callback(currF, currSubPath, isDir)
            if (isDir and result):
                Scan(basePath, currSubPath)
                            
    Scan(path, "")

# -------------------------------------------

def CopyTree(srcPath, dstPath, filter):
    """ filter is f(fullpath) -> bool """
    
    def Callback(fullPath, subPath, isDir : bool):
        
        result = filter(fullPath)
        
        if not result: return False
            
        fullDstPath = os.path.join(dstPath, subPath)
            
        if isDir:
            if not os.path.exists(fullDstPath):
                os.makedirs(fullDstPath)
        else:
            CopyFile(fullPath, fullDstPath)
            
        return True
    
    PathTreeScan(srcPath, Callback)    
    
# -------------------------------------------
    
def InsertEnvVarsInPath(path : str) -> str:
    
    CMarker = '%'
    
    res = path

    while True:
        posBegin = res.find(CMarker)
        if posBegin < 0: break
        posEnd = res.find(CMarker, posBegin + 1)
        assert posEnd >= 0, "Bad env in path " + path
        
        env = res[posBegin + 1:posEnd]
        envVal = os.getenv(env)
        assert envVal is not None, "Can't find env {0} in path {1}".format(env, path)
        res = res[:posBegin] + envVal + res[posEnd + 1:]
        
    res = res.replace('\\', '/')
                            
    return res

# -------------------------------------------

def _PrintFileInfo(name : str):
    
    print( "File: {0}; {1}; {2} days old".format( name, 
                                                  CDUtils.FileDatetime(name), 
                                                  CDUtils.FileDaysOld(name)) )                
    
    
# -------------------------------------------

# forward, any class with Close() method 
class Context: pass

class InstallDesc:
                                                                                                            
    class Item:
        
        def __init__(self, src, dst : str, tags : [str], finalizers = [], initFn = None):
            
            """ init : fn(context : Context, srcRoot : str) -> str """   
            
            assert tags is not None
                             
            self.src = InsertEnvVarsInPath(src) 
            self.dst = InsertEnvVarsInPath(dst) 
            self.tags = tags
            self.finalizers = finalizers
            self.initFn = initFn
                                        
        def __TagAccepted(self, tags : [str]):
            
            for tag in self.tags:
                if tag not in tags: return False
            
            return True

        @staticmethod            
        def __CopyTree(fullName, fullDst):
            
            def f(name : str) -> bool:
                
                fn = os.path.split(name)[1].upper()
                
                if fn in ['.SVN', '.METADATA']: return False
                if fn[-4:] in ['.PYC', '.PY~']: return False
                
                return True       
            
            CopyTree(fullName, fullDst, f)
                    
        def Install(self, context : Context, srcRoot, dstRoot : str, tags : [str]):
            
            if not self.__TagAccepted(tags): return
                                        
            fullName = self.initFn(context, srcRoot) if self.initFn else os.path.join(srcRoot, self.src)
            
            fullDst = os.path.join(dstRoot, self.dst) if self.dst != '/' else dstRoot  
                        
            if os.path.isdir(fullName):             
                self.__CopyTree(fullName, fullDst)                   
            else:
                assert os.path.isfile(fullName), "File not found " + fullName 
                CopyFile(fullName, fullDst)
                
            for f in self.finalizers:  # do finalizers
                f(context, fullName)
                                    
        def __str__(self):
            f = CDUtils.ListClassFields(self)
            return "; ".join(f)
                
                     
    class Tags: pass # forward declaration
                                                    
    def __init__(self, correctTags : Tags, context : Context):
                
        self.correctTags = correctTags 
        self.items = [] # of Item
        self.defaultDst = None
        self.context = context
        
    @property
    def DefaultDestination(self) -> str:
        return self.defaultDst

    @DefaultDestination.setter
    def DefaultDestination(self, dst : str):
        self.defaultDst = dst
        
    def Add(self, src : str, tags : [str] = [], dst : str = None, 
            info = False, initFn = None, finalizeFn = None):
        
        """ initFn, finalizeFn function like finalizerPrintInfo"""
        
        def finalizerPrintInfo(context, srcFullName : str):
            _PrintFileInfo(srcFullName)
                                
        if isinstance(tags, str): tags = [tags]
        for tag in tags: 
            assert self.correctTags.TagCorrect(tag)
        
        if dst is None: 
            assert self.defaultDst is not None
            dst = self.defaultDst
            
        finalizers = []        
        if info:        finalizers.append(finalizerPrintInfo)
        if finalizeFn:  finalizers.append(finalizeFn)
            
        self.items.append( self.Item(src, dst, tags, finalizers, initFn) )
                
    def Install(self, srcRoot, dstRoot : str, tags : [str]):
                
        for i in self.items:
            i.Install(self.context, srcRoot, dstRoot, tags)      
            
        self.context.Close()  
        
    @property
    def Tags(self) -> Tags:
        return self.correctTags

# -------------------------------------------

def ScanTest():
    
    def Inc() -> int:
        nonlocal i
        i += 1
        return i
        
    def Callback(fullPath, subPath, isDir : bool):
        
        print(fullPath, subPath, isDir)
        
        nonlocal i
        
        i += 1
        
        if i > 100: raise Exception()
        
        return True
    
    i = 0    
    PathTreeScan("d:/libs/rtos", Callback)
    
    #Inc()
    #Inc()
    #print(Inc())
    
    
# -------------------------------------------

if __name__ == "__main__":
    
    #ScanTest()
    
    def f(name : str) -> bool:
        
        fn = os.path.split(name)[1].upper()
        
        if fn == '.SVN': return False
        if fn[-4:] == '.PYC': return False
        
        return True       
    
    if sys.platform == "win32":
        CopyTree(r'd:\proj\Ipsius\PyIpsiusQConfig\src', r'd:\temp2\CopyTest', f)
    else:
        CopyTree(r'/home/proj/ipsius/PyIpsiusQConfig/src', r'/home/_temp/CopyTest', f)
        
            