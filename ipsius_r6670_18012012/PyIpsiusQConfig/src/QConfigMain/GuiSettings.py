'''
Created on Jun 16, 2010

@author: root
'''

from PackUtils.CorePathes import TempFiles
from CDUtilsPack.MetaUtils import Property
from QConfigMain.CoreFieldValidator import ValidatorInt, ValidatorTypeErr, ValidatorErr
from PyQt4 import QtGui
from CDUtilsPack.MiscUtils import AssertForException
import os, pickle



class GuiSettings:
    """QConfigMain window startup parameters."""
    
    def __init__(self, fileName : str = "IpsiusQuickConf.conf"):
        self.fileName = TempFiles.File(fileName)
        
        # 'i' in name means 'int' type 
        self.iMainWidth = 960
        self.iMainHeight = 700
        self.iMainX = 0
        self.iMainY = 0
        self.maximized = False
        
        desktop = QtGui.QApplication.desktop()
        self.iMainX = int((desktop.width() - self.iMainWidth) / 2)
        self.iMainY = int((desktop.height() - self.iMainHeight) / 2)
            
        self.validatorInt = ValidatorInt(0)
        
        self.__Validate()
    
    
    FilePath    = Property('fileName')
    
    MainWidth   = Property('iMainWidth', True)
    MainHeight  = Property('iMainHeight', True)
    MainX       = Property('iMainX', True)
    MainY       = Property('iMainY', True)
    IsMaximized = Property('maximized', True)
    
    
    def __Validate(self):
        for key in self.__dict__.keys():
            if key.startswith('i'):
                self.validatorInt.Check(self.__dict__[key], key)
    
    
    def Load(self):
        """Load from file."""
        if not os.path.exists(self.fileName):
            return # use default
        
        def FromFile() -> {}:
            with open(self.fileName, 'r+b') as f:
                res = pickle.load(f)
                assert res
                return res
        
        res = FromFile()
        
        for key in res.keys():
            if key in self.__dict__.keys():
                self.__dict__[key] = res[key]
                  
        self.__Validate()
    
    
    def Save(self):
        """Save to file."""
        if self.MainX < 0: self.MainX = 0 
        if self.MainY < 0: self.MainY = 0
        
        self.__Validate()
        
        data = {}
        for key in self.__dict__.keys():
            if key in ['fileName']: continue
            data[key] = self.__dict__[key]
        
        with open(self.fileName, 'w+b') as f:
            pickle.dump(data, f)


# ------------------------------------------------------------
# Test
# ------------------------------------------------------------

from TestDecor import UtTest
from AllTest.TestRunner import GTestRunner
from AllTest.TestParams import TestParams

@UtTest
def GuiSettingsTest():
    
    def Impl(p : TestParams):        
        s = GuiSettings("TestIpsiusConfig.conf")
        assert s.FilePath
        assert not os.path.exists(s.FilePath)
        
        s.Load()
        assert s.MainWidth == 960
        assert s.MainHeight == 700
        assert s.MainY >= 0
        assert s.MainX >= 0
        assert s.IsMaximized == False
            
        s.MainWidth = 1
        s.MainHeight = 2
        s.MainY = 3
        s.MainX = 4
        s.IsMaximized = True
        s.Save()
        
        s.Load()
        assert s.MainWidth == 1
        assert s.MainHeight == 2
        assert s.MainY == 3
        assert s.MainX == 4
        assert s.IsMaximized == True
        
        s.MainWidth = 'x'
        AssertForException(s.Save, ValidatorTypeErr)
        
        s.MainWidth = -158
        AssertForException(s.Save, ValidatorErr)    
        
        os.remove(s.FilePath)
        
        p.Output("GuiSettingsTest: OK")
        p.Complete()
    
    GTestRunner.RunApp(Impl, traceTest = False)
    
# ------------------------------------------------------------

if __name__ == "__main__":
    
    import unittest
    unittest.main()
    

