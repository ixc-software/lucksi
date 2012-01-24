'''
Created on Apr 7, 2010

@author: root
'''

import sys, os
from CDUtils import CmdOptions
from CDUtilsPack.MetaUtils import UserException
from TestDecor import UtTest

# --------------------------------------------------------------

_CSep               = "="
_CFlagsLineBegin    = "CFLAGS";
_CFlagsLineBegin2   = "CXXFLAGS";
_CFlagsLineEnd      = "$(DEFINES)";
_CLibsLineBegin     = "LIBS"
_CItemSep           = " "

_CInsertedFlags     = ['-Wno-unused-parameter', 
                       '-Wno-unused-variable', 
                       '-Wno-sign-compare']
#                       '-Wno-eof-newline']

# --------------------------------------------------------------

_CFileKey = "file"
_CRemLibsKey = "rem_libs"

# --------------------------------------------------------------

def _RemoveDuplicates(list : []) -> []:
    
    def Next(i : int, list : []) -> bool:
        if i == len(list): return False
        i += 1;
        return True
    
    def RemoveSame(item, list : []) -> []:
        res = [item]
        for i in list:
            if i != item:
                res.append(i)
        return res
    
    res = list
    for i in range(len(list)):
        res = RemoveSame(list[i], res)
        if not Next(i, res): break
    
    return res

# --------------------------------------------------------------------

@UtTest
def RemoveDuplicatesTest() -> None:
    
    def Impl(list : [], checkList : []) -> None:
        checkList.sort()
        res = _RemoveDuplicates(list)
        res.sort()
        assert res == checkList
        
    Impl([5, 1, 2, 3, 1, 1, 2, 3, 4, 1], [1, 2, 3, 4, 5])
    Impl(["1", "2", "3", "", "1", "1"], ["", "1", "2", "3"])
    
    print ("RemoveDuplicatesTest: OK")

# --------------------------------------------------------------
# --------------------------------------------------------------

class MakefileFixer:
    def __init__(self, argv : [str]):
        self.sett = []
        
        self.__Parse(argv)
        self.__Fix()        
        
    def __Parse(self, argv : [str]) -> None:
        argv = argv[1:]
        opts = CmdOptions(autoAddShortNames = True)    
        
        def Add(name : str, rec : bool, varType : str, desc : str) -> None:
            var = "{0}:{1}".format(name, varType)
            opts.Add(name, desc, (var,), rec)
        
        opts.Add(_CFileKey, "Path to makefile with the file name included", 
                 (_CFileKey + ":str",), True)
        opts.Add(_CRemLibsKey, "List of libs to remove", 
                 ('*',), False)
        
        self.sett = opts.Parse(argv, exitOnHelp = True)
        
        # update and check
        if not _CRemLibsKey in self.sett.keys():
            self.sett[_CRemLibsKey] = None
        
        self.sett[_CFileKey] = os.path.abspath(self.sett[_CFileKey])
        if not os.path.isfile(self.File):
            raise UserException("'{0}' is not a file or does not exist"\
                                .format(self.File)) 
    def __Fix(self) -> None:
        data = ""
        with open(self.File, 'r') as f:
            data = f.read()
            lines = data.splitlines()
            
        res = []
        for i in range(len(lines)):
            if (lines[i].find(_CFlagsLineBegin) >= 0 or
                lines[i].find(_CFlagsLineBegin2) >= 0):
                lines[i] = self.__InsertFlags(lines[i], _CInsertedFlags)                
            if lines[i].find(_CLibsLineBegin) >= 0:
                lines[i] = self.__RemoveLibs(lines[i])
        
        with open(self.File, 'w') as f:
            f.write("\n".join(lines))            
    
    def __InsertFlags(self, line : str, flags : [str]) -> str:
        """Insert flags is they don't exist in line."""
        line, sep, tail = line.partition(_CFlagsLineEnd)
        for flag in flags:
            if line.find(flag) < 0:
                line = _CItemSep.join([line, flag])                
        
        return " ".join([line, sep, tail])
    
    def __RemoveLibs(self, line : str) -> str:
        """Simplify line: remove duplicated libs, remove given list of libs."""
        libs = self.sett[_CRemLibsKey]
        if not libs: return line
        
        head, sep, line = line.partition(_CSep)
        items = line.split(_CItemSep)
        #items = _RemoveDuplicates(items)
        
        for lib in libs:
            while lib in items: 
                items.remove(lib)
        
        sorted = _CItemSep.join(items)
        
        return _CItemSep.join([head + sep, sorted])
    
    @property
    def File(self) -> str:
        return self.sett[_CFileKey]
        
# --------------------------------------------------------------

@UtTest
def MakefileFixerTest() -> None:
    file = "./Test_Makefile"
    data = """
        CXX           = g++
        CFLAGS        = -pipe -O2 -D_REENTRANT -Wall -W -Wno-unused-variable $(DEFINES)
        CXXFLAGS      = -pipe -O2 -D_REENTRANT -Wall -W -Wno-sign-compare $(DEFINES)
        LINK          = g++
        LFLAGS        = -rdynamic -Wl,-O1 -Wl,-rpath,/usr/local/Trolltech/Qt-4.6.0-static/lib
        LIBS          = $(SUBLIBS) -L/usr/local/Trolltech/Qt-4.6.0-static/lib /lib/libsrtp.a /lib/libiberty.a -pthread -pthread $(QTDIR)/lib/libQtCore.a -pthread -lQtCore -lQtNetwork -lQtGui -lm 
        RANLIB        = 
        QMAKE         = /usr/local/Trolltech/Qt-4.6.0-static/bin/qmake"""
    
    remLibs = ('-lQtCore', '-lQtGui', '-lQtNetwork',
               '$(QTDIR)/lib/libQtCore.a', '-pthread')
    
    with open(file, 'w') as file:
        file.write(data)
    
    cmdLine = ["self", "/f", "./Test_Makefile", 
                        "/r"]
    for l in remLibs:
        cmdLine.append(l)
    
    mf = MakefileFixer(cmdLine)
    
    checkData = ""
    with open(mf.File, 'r') as file:
        checkData = file.read()
    
    os.remove(mf.File)
    
    print (checkData)
    
    for item in _CInsertedFlags:
        assert checkData.find(item) >= 0
    
    for item in remLibs:
        assert checkData.find(item) < 0
        
    print ("MakefileFixerTest: OK")

# --------------------------------------------------------------

if __name__ == "__main__":
    
    #RemoveDuplicatesTest().Call()
    #MakefileFixerTest().Call()
        
    try:
        mf = MakefileFixer(sys.argv)
        print ("Makefile '{0}' updated.".format(mf.File))
        
    except UserException as e:
        print (e)

    
        
    
    
    
    