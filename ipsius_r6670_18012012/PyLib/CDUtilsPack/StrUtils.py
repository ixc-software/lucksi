
import io
import os
import unittest

from collections import Iterable

from TestDecor import UtTest

# -----------------------------------------------------     

def IntToHex(val : int, bytes = 4, prefix = True) -> str:
    s = "{0:0>#" + str(2 + bytes * 2) + "x}"
    s = s.format(val)
    if not prefix: s = s[2:]
    return s

# -----------------------------------------------------     

def DumpBytes(b : bytes) -> str:
    return " ".join( [IntToHex(x, 1, prefix = False) for x in b] )

# -----------------------------------------------------     

def IntToSize(val : int, align : int = 0) -> str:
    s = str(val)
    res = ''
    
    while True:
        if len(s) > 3:
            res = "'{0}{1}".format(s[-3:], res)
            s = s[:-3]
        else:
            res = s + res
            break
                            
    return AlignStr(res, align)

# -----------------------------------------------------     

# use negative width for align s left
def AlignStr(s : str, width : int, fill = " ") -> str:
    diff = abs(width) - len(s)
    if diff <= 0: return s    
    diff = fill * diff
    return diff + s if width > 0 else s + diff   

# -----------------------------------------------------     

#  end is always excluded
def StringCut(s, beg, end : str, fn : "str -> str" = None, excludeBeg = False) -> [""]:
        
    result = []    
    currPos = 0
    
    while True:
        pos = s.find(beg, currPos)
        if pos < 0: break
        
        posEnd = s.find(end, pos + len(beg))
        if posEnd < 0: break
        
        if excludeBeg: pos += len(beg)
        
        item = s[pos:posEnd]
        currPos = posEnd + len(end)
        
        if fn != None:
            item = fn(item)
            if item == None: continue 
        
        result.append(item)
    
    return result

# -----------------------------------------------------     

class StrTinyTests(unittest.TestCase):
    
    def test_misc(self):
        
        _a = self.assertEqual
        
        _a( IntToHex(0x10a, 1),                 "0x10a" )
        _a( IntToHex(0xfe, 2, prefix = False),  "00fe" )
        
        _a( DumpBytes(b"\x10\x1f\xff"),         "10 1f ff" )
        
        _a( IntToSize(15109901),                "15'109'901" )
        _a( IntToSize(15, align = 4),           "  15" )
        
        _a( AlignStr("15", 4, "-"),             "--15" )
        _a( AlignStr("10", -4),                 "10  " )
        
    def test_StringCut(self):
        
        s = "blah blah <b>text</b> another <b>text2</b>"
        
        res = StringCut(s, "<b>", "</b>", lambda s: s.upper(), excludeBeg = True)
        
        self.assertEqual(res, ["TEXT", "TEXT2"])        


# -----------------------------------------------------     

def SaveStringList(list : [str], fileName : str, encoding = "utf_8", lineFeed = '\n') -> None:
    
    assert isinstance(list, Iterable)
    assert isinstance(fileName, str)
    
    with io.open(fileName, mode = "w+t", encoding = encoding) as f:  
        for line in list:
            f.write(line)
            f.write(lineFeed)

# -------------------    

def LoadStringList(fileName : str, encoding = "utf_8", linesLimit = 0, joinWith = '') -> [str] or str:
    
    res = []
    count = 0
    
    with io.open(fileName, mode = "rt", encoding = encoding) as f:
        for line in f:
            line = line.strip('\x0d\x0a')
            res.append(line)
            count += 1
            if (linesLimit > 0 and count >= linesLimit): break
            
    if len(joinWith): 
        res = joinWith.join(res)            
            
    return res 

# -------------------    

@UtTest
def Test_LoadSaveStringList():
    
    CFile = "string_list_test.txt"
    
    sl = ["1", "2", "3"]
    SaveStringList(sl, CFile)
    
    slLoaded = LoadStringList(CFile)
    
    assert sl == slLoaded
    
    os.remove(CFile)

# -----------------------------------------------------

""" 
    Split command line like:
    "a -b c d -e -f g".split(" ") -> ["a"], ["-b", "c", "d"], ["-e"], ["-f", "g"]
"""                
def SplitCommandLineByKeys(opts : [str]) -> [[str]]:
    keys = [i for i, o in enumerate(opts) if o.startswith('-')]
    keys.append(len(opts)) # for last key slice
    if keys[0] != 0: keys.insert(0, 0)
    return [ opts[keys[i]:keys[i+1]] for i in range(len(keys)-1) ]
    
def SplitStringToCommandLineList(s : str, quote : str = "'") -> [str]:    
    res = []    
    inQuotes = False
    startPos = 0 
    for i, c in enumerate(s):
        if c == quote: inQuotes = not inQuotes
        atEnd = (i == len(s) - 1)
        if (c == " " and not inQuotes) or atEnd:
            res.append(s[startPos:i + 1 if atEnd else i].strip(quote))
            startPos = i + 1 
    
    return res

@UtTest                                
def Test_CmdLineTools(silence : bool = True):
    s = "a 'fuck them all' -b c d -e -f g"
    sSplit = SplitStringToCommandLineList(s)
    sByKeys = SplitCommandLineByKeys(sSplit)
    
    assert sSplit == ['a', 'fuck them all', '-b', 'c', 'd', '-e', '-f', 'g']
    assert sByKeys == [['a', 'fuck them all'], ['-b', 'c', 'd'], ['-e'], ['-f', 'g']]  
    
    if not silence:
        print(s, sSplit, sByKeys, sep = '\n')
		
# -----------------------------------------------------

class ConvertErr(Exception):
    def __init__(self, typeFrom : str, typeTo : str, text : str):
        Exception.__init__(self, "Can't convert {0} to {1}: \"{2}\"".
                                 format(typeFrom, typeTo, text))

# ---------------

def ToBytes(text : str) -> bytes: 
    return text.encode('ascii')

# ---------------

def ToStr(byteData : bytes) -> str: 
    return byteData.decode('ascii')
	
# ---------------
	
def StrToBool(text : str) -> bool:
    if text == "1" or text.upper() == "TRUE": return True
    if text == "0" or text.upper() == "FALSE": return False
    
    raise ConvertErr("str", "bool", text)
	
# ---------------
	
def BoolToStr(val : bool) -> str:
    if val: return "True"
    else: return "False"
	
# ---------------
	
def StrToInt(text : str) -> int:
    try:
        return int(text)
    except ValueError:
        raise ConvertErr("str", "int", text)
		
# ---------------
		
def IntToStr(val : int) -> str:
    return "{}".format(val)

# ---------------

def Unquote(data : str) -> str:
    if data.startswith('"') and data.endswith('"'):
        return data[1:-1]
    return data

# -----------------------------------------------------

if __name__ == "__main__":
    
    unittest.main()
    
    pass



