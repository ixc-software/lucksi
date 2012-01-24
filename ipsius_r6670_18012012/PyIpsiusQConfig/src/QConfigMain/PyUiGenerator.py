'''
Created on Feb 10, 2010

@author: root
'''
from CDUtilsPack.MiscUtils import GetScriptDir, AssertForException

import os, glob, sys
from CDUtils import QuickFileCache
from PackUtils.CorePathes import TempFiles 
from PackUtils.CoreBaseException import CoreBaseException

class GeneratorError(CoreBaseException):
    pass

# -----------------------------------------------------

def _FileIsSame(path : str, cacheFilePath : str) -> bool:
    #dirFile = os.path.split(file)
    #fileName = "{0}_{1}".format(os.path.split(dirFile[0])[1], dirFile[1])
    file = os.path.basename(path)
    dir = os.path.basename(os.path.dirname(path))
    file = "{0}_{1}.cache".format(dir, file)
    cacheFile = os.path.join(cacheFilePath, file)
    
    res = QuickFileCache.VerifyCache(cacheFile)
    if not res:
        QuickFileCache.Save(path, cacheFile)        
    return res

# -----------------------------------------------------

def _FindAllUi(dir : str) -> [str]:
    res = []
    for dir, subDirs, files in os.walk(dir):
        for file in files:       
            if file.endswith(".ui"):
                res.append(os.path.join(dir, file))
    return res
                
# -----------------------------------------------------

def Generate(srcDir : str, destDir : str, newFilePrefix = "gen", 
             traceFn : lambda str : None = print) -> bool:
    """Generate .py files from .ui files in destDir recursively."""
    res = False
    files = _FindAllUi(srcDir)
    
    if not files: 
        raise GeneratorError("No '*.ui' files found.")
    
    for uiFile in files:
#        uiPathName = os.path.split(uiFile)
#        dirName = os.path.split(uiPathName[0])[1]
#        pyFileName = "{0}_{2}_{1}.py".format(newFilePrefix, uiPathName[1][:-3],
#                                           dirName)
        fileName = os.path.basename(uiFile)
        dirName = os.path.basename(os.path.dirname(uiFile))
        pyFileName = "{0}_{2}_{1}.py".format(newFilePrefix, fileName[:-3],
                                           dirName)
        pyFile = os.path.join(destDir, pyFileName) 
        if _FileIsSame(uiFile, destDir) and os.path.exists(pyFile): continue
        
        res = True
        if traceFn:
            traceFn("Generate python module '{0}' for '{1}' from '{2}'".\
                     format(os.path.basename(pyFile), fileName, dirName))
        
        execFile = GetScriptDir("../QConfigMain/_PyUiGeneratorDetail.py")
        
        cmd = '"{0}" "{1}" -o "{2}" "{3}"'.format(sys.executable, execFile, pyFile, uiFile)
        if (sys.platform == 'win32'): cmd = '"' + cmd + '"' 
         
        if os.system(cmd) !=  0: 
            raise GeneratorError("Failed to generate .py file.")        
    
    return res
        
# -----------------------------------------------------
# Test
# -----------------------------------------------------

from TestDecor import UtTest
from AllTest.TestRunner import GTestRunner

@UtTest
def PyUiGeneratorTest():
    
    def Impl(outputFn):
        destDir = TempFiles.DirUi
        genUi = TempFiles.UiFile("PyUiGenerator_Test.ui")
        with open(genUi, "w") as f:
            f.write("<?xml version=\"1.0\" encoding=\"UTF-8\"?> \n\
                     <ui version=\"4.0\"> \n\
                     <class>Dialog</class> \n\
                     <widget class=\"QDialog\" name=\"Dialog\">\n\
                     </widget>\n\
                     <resources/>\n\
                     <connections/>\n\
                     </ui>")
        
        Generate(GetScriptDir("../"), destDir, "test_gen", print)
        
        fn = lambda: Generate(GetScriptDir("../tmp"), destDir, "test_gen")
        AssertForException(fn, GeneratorError)
        
        os.remove(genUi)
        
        outputFn("PyUiGeneratorTest: OK")
        
    
    GTestRunner.Run(Impl, traceTest = False, preRunSetupWithUi = False)
    
# -----------------------------------------------------

if __name__ == "__main__":
    
    import unittest
    unittest.main()
        
    
        
        