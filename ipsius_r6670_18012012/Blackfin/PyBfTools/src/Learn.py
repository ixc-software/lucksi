'''
Created on 27.11.2009

@author: Alex


---

ListClassFields -- complete; make test with props, [obj], dict etc.  
  str, list, tuples (test as sequence classes) -> recursive fn for print 
  specail output for dicts and list values (use str()) + recursive 
  re-order - ? 
  private, properties filter - ? 
  suppress "" and None -- optional 

Py int math benchmark vs c++

Note:
Py class is just dict; instance is class dict clone + call __init__
@property work thru Descriptors protocol (how? setters? etc.); 
try emulate thru custom __getattr__ + __dir__


'''

import io
import os.path
import time
import pickle
import collections
import copy
import unittest

from numbers import Number
from types import FunctionType
from collections import Iterable 

import CDUtils
from TestDecor import UtTest

# -----------------------------------------------------     
  
def _RecMax(a : [Number]) -> Number:

    def f(max, i : Number) -> Number:
        if i >= len(a): return max
        if a[i] > max: return f(a[i], i+1)
        return f(max,  i+1)

    return f(a[0], 1)

# -----------------------------------------------------     

class _A: pass  # forward delcaration

class _A:
    def __init__(self, val):
        self.m_val = val
        
    def Add(self, other : _A):
        self.m_val += other.m_val
        
    def __str__(self):
        return str(self.m_val)

# -----------------------------------------------------     

def _SliceBench():
    
    import time
    import types
    import copy
     
    lx = list(range(1, 512 * 1024))
    
    def SumRaw() -> int:
        return sum(lx)

    def SumSlice() -> int:
        return sum(lx[:])
    
    def Copy(lx):
        a = copy.copy(lx)
        
    assert(SumRaw() == SumSlice())
    
    def BenchLoop(timesToRun : int, fn : types.FunctionType, *args) -> float:
        t = time.clock()
        for i in range(timesToRun): fn(*args)
        return time.clock() - t
        
    CTimesToRun = 20
    print("Started...")
    print(BenchLoop(CTimesToRun, SumRaw) / CTimesToRun)
    print(BenchLoop(CTimesToRun, SumSlice) / CTimesToRun)
    print(BenchLoop(CTimesToRun, Copy, lx) / CTimesToRun)

# -----------------------------------------------------     

"""

*** PyEnum ***

Options:    
    - allow or not dublicates
    - make bit mask
        
***    
           
Make test by asserts and silence mode     
Py function nested scopres: not locals and globals, how to access it (frame - ?) 
 

"""

import PyEnum     

CYellowOffs = 1024

prevGlobals = globals()
    
@PyEnum.EnumClass
def Color():
    clNone
    
    clRed   = 0x001    
    clGreen = 0x010     
    clBlue  = 0x100
    
    clYellow = CYellowOffs + clRed + clGreen  # calc it, capture extern value
    clYellowNext                              # clYellow + 1
    clRedClone = clRed

# verify what PyEnum.EnumClass don't change globals()     
assert globals() == prevGlobals
del prevGlobals

""" test global enum """
def EnumTest(silentMode : bool):
    
    # suppress print
    if silentMode: print = lambda *args: None
    else: print = __builtins__.print

    # read-only verify    
    def AssertForException(fn, exceptionType):
        wasException = False
        try:
            fn()
        except exceptionType:
            wasException = True
        assert(wasException)
    
    def f(): Color.clGreen = 10
    AssertForException(f, PyEnum.EnumClassException)
    def f(): del Color.clRed
    AssertForException(f, PyEnum.EnumClassException)

    # values verify
    assert Color.clGreen.Value == 0x010
    assert Color.clRed.Value == 0x001
    assert Color.clYellowNext.Value == Color.clYellow.Value + 1

    # print test
    print(Color.clGreen)    
    print(Color)
    
    for k in Color:
        print(Color[k].Name, '=', Color[k].Value)  # or Color[k]
    print()

    # test ValueToItems()
    def ValueToItemsTest(val : int, correctResult):
        
        def EnumItemsToStr(items):
            return '(' + ', '.join([str(i) for i in items]) + ')'
        
        assert correctResult == Color.ValueToItems(val)
        
        print(val, '->', EnumItemsToStr(correctResult))
    
    ValueToItemsTest(1, (Color.clRed, Color.clRedClone))
    ValueToItemsTest(-1, tuple())
    ValueToItemsTest(0x10, (Color.clGreen,))

    # typesafety test
    assert Color.VerifyItem(Color.clGreen)
    assert not Color.VerifyItem('15', False)
    
    print('')
    
""" test local enum """    
def EnumTest2(silentMode : bool):

    if silentMode: print = lambda *args: None
    else: print = __builtins__.print
    
    v = 1
        
    @PyEnum.EnumClass        
    def x():
        one = v
        two = v + 10
        three
                
    print(x)
    print(x.one)

# -----------------------------------------------------     

def ContextTest():

    v = 1
    
    class B:
        One = v
    
    s = 'class A:\n\tOne = v\n'
#    s = 'print(v)'

    import copy
    def SuperCopy(globs, locs):
        g = copy.copy(globs)
        for k in locs: g[k] = locs[k]
        return g
    
    g = SuperCopy(globals(), locals())
    print(g)    
    exec(s, g)
    print(globals())

# -----------------------------------------------------     

# Pythonofucks

# fuck #1        
def Locals1():
    src = ['a = 10', 'def f(): pass']
    exec('\n'.join(src))
    
    # locals contains 'a' and 'f'    
    print(locals())
    
    # ok
    print(eval('a'))
    print(eval('f()'))

    # fail
    print(a)    
    f()
    
# fuck #2
def Locals2(silenceMode):
    if silenceMode: print = lambda *a, **k: None
    
    print('Hello', 'world!')
    
# fuck #3
def Locals3():
    
    a1 = 10
    a2 = 10
    
    src = ['b = a1  # ok',
           'class A:',
           '    F = a2 # fail']
    
    exec('\n'.join(src))    

# -----------------------------------------------------     

def RunEnumTests():
    silentMode = False 
    EnumTest(silentMode)
    EnumTest2(silentMode)

# -----------------------------------------------------     

def BindTest():
    
    class A:
        def __init__(self):
            self.val = 0
            
        def Add(self, val) -> int:
            self.val += val
            return self.val
            
    def Bind(self, f):
        
        def fn(*arg):
            return f(self, *arg)
        
        return fn
        
    a = A()
    f = Bind(a, a.__class__.Add)  # a.Add  
    print(f(10))
    print(f(10))
    
    print(a.Add)
    print(a.__class__.Add)
    print(A.__dict__["Add"])
    
# -----------------------------------------------------     

def _ThreadTest():
    
    import threading
    
    def Body():
        print("inside-thread", threading.current_thread().name)
            
    print(threading.current_thread().name)
    t = threading.Thread(target = Body, name = "my")
    print("Run")
    t.start()
    
    print("Done!")
    
def ThreadTest():
    
    import threading, time
    
    CFile = r"d:\_Torrent\06\boxing\Joel Casamayor vs Juan Manuel Marquez HBO PPV 9-13-2008.avi"
    CBlockSize = 512 * 1024 # 32 * 1024 * 1024
        
    def SingleAction():
        sum = 0
        for i in range(2 * 1024):
            sum += i
        return sum
    
    def Benchmark(durationSec):
        t = time.clock()
        i = 0
        while time.clock() - t < durationSec:
            SingleAction()
            i += 1
        t = time.clock() - t
        return i / t
    
    def LoadFile():
        nonlocal readDone
        
        size = 0
        data = bytearray(CBlockSize * b'\x00')
        with io.open(CFile, 'br') as f:
            while True:
                lx = f.readinto(data)
                size += lx
                if lx != len(data): break
        
        print("Size", size)    
        readDone = True
        
    
    # bench
    print('Start...')
    print(Benchmark(1.0))  # ~2'100
    
    # thread test
    readDone = False

    t = time.clock()
    thread = threading.Thread(target = LoadFile)
    thread.start()
    
    i = 0
    while not readDone:
        SingleAction()
        i += 1        
        time.sleep(0)
        
    if i == 0: i = 1
        
    t = time.clock() - t
    
    print('Time ', t, '; count', i, '; rate ', i / t)

# -----------------------------------------------------     

def AroundMax():
    strList = ["a", "abcd", "b"]
    maxLen = 4  # len("abcd")
    
    # 1
    assert len( max(strList, key = len) ) == maxLen 
    
    assert len( max(strList, key = lambda s: len(s)) )  == maxLen  # same
    
    # 2
    assert max( map(len, strList) ) == maxLen
    
    def strLenGen(): 
        for s in strList: yield len(s)
    assert max( strLenGen() ) == maxLen   # same
        
    # 3
    assert max([len(s) for s in strList]) == maxLen  
    

# -----------------------------------------------------     

def FnDoc(s : str):
    
    """ Decorator, set __doc__ attribute for function or class """
    
    def f(obj):
        assert isinstance(obj, FunctionType)
        obj.__doc__ = s
        return obj
    
    return f

class SeqDiff:

    def __init__( self, leftSeq, rightSeq, key = lambda i1, i2: i1 == i2 ):
        
        def Find(item, seq) -> int:
                        
            for indx, i in enumerate(seq):
                if key(item, i): return indx
                            
            return -1
                
        rightSeq = copy.copy(rightSeq) # copy, 'couse we change rightSeq
        
        self.uniqLeft = []
        self.both = [] 
        
        for i in leftSeq:
            rightIndx = Find(i, rightSeq)
             
            if rightIndx >= 0:
                self.both.append(i)
                del rightSeq[rightIndx] 
            else:
                self.uniqLeft.append(i)
                
        self.uniqRight = rightSeq        

    @property
    def UniqLeft(self) -> []: return self.uniqLeft
    
    @property
    def UniqRight(self) -> []: return self.uniqRight
    
    @property
    def Both(self) -> []: return self.both

    @FnDoc("Print diff result")     
    def Print(self, formatFn = str):
        assert False and "Unimplemented"
    
    @staticmethod
    def Test():
        left = [0, 1, 2, 3]
        right = [2, 3, 4]    
        diff = SeqDiff(left, right)
        
        assert diff.UniqLeft == [0, 1]
        assert diff.UniqRight == [4]
        assert diff.Both == [2, 3]
        
        
@FnDoc("""Bin/text reader diff

...more help
""")         
def OpenView():

    SeqDiff.Test()
    
    def PrintDataType(f):
        for i in f:
            print(type(i))
            break

    CFile = 'c:/a.bin'
            
    with io.open(CFile, 'rb') as f:
        bin = dir(f)
        PrintDataType(f)
        
    with io.open(CFile, 'rt') as f:
        text = dir(f)
        PrintDataType(f)
            
    diff = SeqDiff(bin, text)
    print("Bin", diff.UniqLeft)
    print("Text", diff.UniqRight)
    print("Both", [s for s in diff.Both if not s.startswith('__')])
        

# -----------------------------------------------------     

#a = "global"        
#
#class Spam:
#
#    a = "in spam"
#    
#    def eggs(self):
#        b = a   # get "global"
#        print (b)
#
#
#Spam().eggs()

"""
    Coverage
    Property
    RegExp speed
"""

# -----------------------------------------------------     

# TODO -- make command lile tool
def CoverageTest():
    
    import coverage.cmdline
    
    cs = coverage.cmdline.CoverageScript().command_line

    CFile = r"d:\proj\Ipsius\PyBootTest\src\BootToolsMain.py"
    CFile = r"d:\proj\Ipsius\Blackfin\PyBfTools\lib\CDUtils.py"
    
#    Cv.CoverageScript().command_line( ["run", CFile] ) 
#    cs( ["report", "-m"] ) 
    cs( ["html", "-d", "cov_report"])


# -----------------------------------------------------     

                
if CDUtils.IsMain():
    
    pass
    
