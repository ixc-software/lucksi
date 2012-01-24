'''
Created on 16.02.2010

@author: Alex
'''

import random
import time
import re

import CDUtils

# -------------------------------------------------------------

def RegExpBench():
    
    CNull = b'-'
    CMByte = 1024 * 1024
    CSize = 64 * CMByte
    CStr1 = b'1234'
    CStr2 = b'123x666'
    
    def MakeData():
        data = bytearray(CNull * CSize)
        
        def AddPatch(s : bytes):
                            
            while True:
                pos = random.randint(0, len(data) - 1 - len(s))                                    
                i = map( lambda x: x == ord(CNull), data[pos:pos + len(s)] )                            
                ok = all(i)
                if not ok: continue
                
                for i, c in enumerate(s):
                    data[pos + i] = c
                break
                            
        AddPatch(CStr1)
        AddPatch(CStr2)
                
        return data 
    
    def RawFind(data : bytes, *find : bytes) -> [int]:
        
        res = []

        def Find(s : bytes):
            
            currPos = 0
            
            while True:
                n = data.find(s, currPos)
                if n < 0: break
                res.append(n)
                currPos = n + len(s) 
        
        for s in find:
            Find(s)
        
        return res
    
    def RegExpFind(data : bytes, *find : bytes) -> [int]:
        pattern = b"|".join(find)
        res = re.finditer(pattern, data)
        return [m.start() for m in res]
            
    def FindBench(f):
        t = time.clock()

        for k in range(10):
            res = f(data, CStr1) # CStr2
        
        t = time.clock() - t
        
        rate = len(data) / CMByte / t
        print(f.__name__, format(rate, ".2f"), [CDUtils.IntToSize(i) for i in res])
        
    data = MakeData() # make data

    FindBench(RawFind)    
    FindBench(RegExpFind)    

# -------------------------------------------------------------

def MutableTest():
    
    def Case1():
        b = b'a'
        a = b
        b += b'b'
        return a
    
    def Case2():
        b = bytearray(b'a')
        a = b
        b += b'b'
        return a
        
    print( Case1() )
    print( Case2() )
    
# -------------------------------------------------------------

if __name__ == '__main__':
    
#    RegExpBench()

    MutableTest()
    
    
    
    pass
    