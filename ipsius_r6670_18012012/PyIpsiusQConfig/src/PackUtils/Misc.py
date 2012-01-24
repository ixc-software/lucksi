'''
Created on Jan 27, 2010

@author: root
'''

from PyQt4 import QtCore
from PackUtils.CoreBaseException import CoreBaseException

import CDUtilsPack.CheckFnArgs
from CDUtilsPack import StrUtils

# --------------------------------------------------------------------

#class ConvertErr        
#def ConvDecorator(fn): # rehandle exception
 
# --------------------------------------------------------------------

CheckFnArgs = CDUtilsPack.CheckFnArgs.CheckFnArgs

ConvertErr = StrUtils.ConvertErr
ToBytes = StrUtils.ToBytes
ToStr = StrUtils.ToStr
StrToBool = StrUtils.StrToBool
BoolToStr = StrUtils.BoolToStr 
StrToInt = StrUtils.StrToInt
IntToStr = StrUtils.IntToStr
Unquote = StrUtils.Unquote

# --------------------------------------------------------------------

def NotImplemented():
    assert 0 and "Not implemented!"

# --------------------------------------------------------------------

def QProcessErrorToStr(e : QtCore.QProcess.ProcessError) -> str:
    errors = ["QtCore.QProcess.FailedToStart",
              "QtCore.QProcess.Crashed", 
              "QtCore.QProcess.Timedout", 
              "QtCore.QProcess.WriteError",
              "QtCore.QProcess.ReadError",
              "QtCore.QProcess.UnknownError"]
     
    res = (errors[e] if (int(e) in range(len(errors))) 
                     else "Unknown 'QtCore.QProcess.ProcessError' value.") 
    return res    
    

# --------------------------------------------------------------------
# Tests
# --------------------------------------------------------------------

from TestDecor import UtTest


@UtTest
def CheckFnArgsTest():
    
    from AllTest.TestRunner import GTestRunner
    from CDUtilsPack.CheckFnArgs import CheckFnArgsTestImpl         
    
    def Impl(outputFn):                   
        CheckFnArgsTestImpl()
        outputFn("CheckFnArgsTest: OK")    
    
    GTestRunner.Run(Impl, traceTest = False)

# --------------------------------------------------------------------

if __name__ == "__main__":
    
    import unittest
    unittest.main()