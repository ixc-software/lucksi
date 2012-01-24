'''
Created on Jun 1, 2010

@author: root
'''

import os
import time
from PyQt4 import QtGui
import glob, unittest
from CDUtils import GetScriptDir
from ModulesMng import TestSuiteForModule, TestSuiteForPackage

# ------------------------------------------------------

def _RunTests(fileList : [str], outputFn) -> bool:
    for name in fileList:
        name = name[:name.rfind('.')]
        outputFn(name, "start")
        test = TestSuiteForModule(name, -1)
        # time.sleep(0.5)
        res = unittest.TextTestRunner().run(test)
        outputFn(name, "end")
        if len(res.errors) > 0 or len(res.failures) > 0: return False
    return True

# ------------------------------------------------------

def RunAllTestsOneByOne():
    print("Running tests ... ")
    
    def RunFor(moduleName : str, exclude : [str] = None):
        files = glob.glob(os.path.join(GetScriptDir("../" + moduleName), "*.py"))
        tests = []
        Add = lambda name: tests.append(moduleName + "." + name)
        
        for file in files:
            name = os.path.basename(file)
            
            if name.startswith("_"): continue
            
            if exclude:
                skip = False
                for item in exclude:
                    if item in name: 
                        skip = True
                        break
                if not skip: Add(name)
            else:
                Add(name)     
        
        assert _RunTests(tests, print)
    
    mainExclude = []
    testExclude = [] 
    RunFor("Main", mainExclude)
    RunFor("Test", testExclude)
    
    if not testExclude and not mainExclude:
        print("All tests are OK")
        return
    
    l = []
    if mainExclude: l += mainExclude
    if testExclude: l += testExclude
    print("Tests: {} are skipped. Other tests are OK".format(l))
    
    
    
    
        
# ------------------------------------------------------

def RunAllTestsViaSuit():
    
    def ForPackage(name : str):
        #print ("\nRunning tests for module '{}'".format(name))
        test = TestSuiteForPackage(name, 2, True)
        #print(unittest.TextTestRunner().run(test))
    
    ForPackage("AllTest")
    ForPackage("QConfigMain")

# ------------------------------------------------------

def RunAllTests():
    RunAllTestsViaSuit()
    #RunAllTestsOneByOne()
    
# ------------------------------------------------------

if __name__ == "__main__":
    
    RunAllTests()


    
    
    