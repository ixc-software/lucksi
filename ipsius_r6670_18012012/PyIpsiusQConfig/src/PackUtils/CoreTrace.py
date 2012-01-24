'''
Created on Jun 2, 2010

@author: root
'''

import CDUtilsPack.CoreTrace

# ----------------------------------------------

#Class - wrapper for trace function and trace state (enable/disable).
CoreTrace = CDUtilsPack.CoreTrace.CoreTrace 

# ----------------------------------------------
# Test
# ----------------------------------------------

from TestDecor import UtTest

@UtTest
def CoreTraceTest():
        
    def Impl(outputFn):
        CDUtilsPack.CoreTrace.CoreTraceTestImpl()        
        outputFn("CoreTraceTest: OK")
    
    from AllTest.TestRunner import GTestRunner
    GTestRunner.Run(Impl, traceTest = False)

# ----------------------------------------------

if __name__ == "__main__":
    
    import unittest
    unittest.main()