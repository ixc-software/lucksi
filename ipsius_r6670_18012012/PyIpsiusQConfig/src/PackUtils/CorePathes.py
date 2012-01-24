'''
Created on Feb 4, 2010

@author: root
'''
from CDUtilsPack import MiscUtils
from CDUtilsPack.MetaUtils import Property
from CDUtilsPack.MiscUtils import LoadModuleFromFile
import os.path


# ------------------------------------------------------------

def _Dir(name : str, level : str = "../../") -> str:
    return MiscUtils.GetScriptDir(level + name)

_Join = os.path.join

# ------------------------------------------------------------

class TempFiles:
    Dir         = _Dir("tmp")
    
    DirScript   = _Join(Dir, "isc")
    DirUi       = _Join(Dir, "ui")
    DirLog      = _Join(Dir, "log")
    DirTest     = _Join(Dir, "test")
    
    
    @staticmethod
    def File(name : str) -> str:
        """Return absolute path for file in directory TempFiles.Dir"""
        return _Join(TempFiles.Dir, name)
    
    
    @staticmethod
    def ScriptFile(name : str) -> str:
        """Return absolute path for file in directory TempFiles.DirScript"""
        return _Join(TempFiles.DirScript, name)
    
    
    @staticmethod
    def UiFile(name : str) -> str:
        """Return absolute path for file in directory TempFiles.DirUi"""
        return _Join(TempFiles.DirUi, name)
    
    
    @staticmethod
    def LogFile(name : str) -> str:
        """Return absolute path for file in directory TempFiles.DirLog"""
        return _Join(TempFiles.DirLog, name)
    
    
    @staticmethod
    def TestFile(name : str) -> str:
        """Return absolute path for file in directory TempFiles.DirTest"""
        return _Join(TempFiles.DirTest, name)
    
    
    @staticmethod
    def TestDir(name : str) -> str:
        """Create directory with name = 'name' in directory 
        TempFiles.DirTest and return absolute path for it."""
        dir = _Join(TempFiles.DirTest, name)
        if not os.path.exists(dir): os.makedirs(dir)
        assert os.path.exists(dir)
        return dir
    
    
    @staticmethod
    def UiPyModule(deviceFolderName: str, baseUiFileName = 'Widget'):
        """
        'deviceFolderName' - name of folder in './src/Devices': 'Test', 'E1', etc,
        'baseUiFileName'   - name of .ui file in folder 'deviceFolderName',
        Return py-module.
        """
        file = "gen_{0}_{1}.py".format(deviceFolderName, baseUiFileName)
        return LoadModuleFromFile(TempFiles.UiFile(file))        
    
    
    @staticmethod
    def MakeDirs():
        d = TempFiles.__dict__
        for key in d:
            if key.startswith("Dir"):
                val = str( d[key] )
                if not os.path.exists(val): 
                    os.makedirs(val)
    
    
# ------------------------------------------------------------

class IconFiles:
    Dir     = _Dir("images")
    
    Main    = _Join(Dir, "app.png")
    Run     = _Join(Dir, "run.png")
    Stop    = _Join(Dir, "stop.png")
    Save    = _Join(Dir, "save.png")
    Undo    = _Join(Dir, "undo.png")
    Default = _Join(Dir, "default.png")
    Monitor = _Join(Dir, "monitor.png")

# ------------------------------------------------------------

class TestFiles:
    Dir             = _Dir("AllTest", "../")    
    ScriptFile      = _Join(Dir, "TestScript.isc")
    ConfigFile      = _Join(Dir, "TestConst.py")
    
    @staticmethod
    def File(name : str) -> str:
        """Return absolute path for file in directory TestFiles.Dir"""
        return _Join(TestFiles.Dir, name)

# ------------------------------------------------------------

if __name__ == "__main__":
    
    print (TempFiles.Dir)
    print (TempFiles.File("x.txt"))
    print (TempFiles.ScriptFile("x.isc"))
    print (TempFiles.UiFile("x.ui"))
    print (TempFiles.TestFile("x.test"))
    TempFiles.MakeDirs()
    
    print (IconFiles.Main)
    
    print (TestFiles.Dir)
    print (TestFiles.File("x.test"))
    print (TestFiles.ScriptFile)
    
    
