'''
Created on Jun 2, 2010

@author: root
'''

from CDUtilsPack.CheckFnArgs import CheckFnArgs

# ----------------------------------------------

class CoreTrace:
    """
    Class - wrapper for trace function and trace state (enable/disable).
    """
    
    def __init__(self, fn, enable : bool):
        """
        'fn'     - fn(obj, text : str, textID : str) -> None, 
        'enable' - using to enable / disable trace in runtime.
        """
        self.enable = enable
        self.fn = fn
        CheckFnArgs(self.fn, (None, str, str), None)        
    
    def Add(self, obj, text : str, textID : str = None):
        if not self.enable: return
        self.fn(obj, text, textID)
    
    def Enable(self, state : bool):
        self.enable = state
    
    def Copy(self): # -> CoreTrace
        return CoreTrace(self.fn, self.enable)

# ----------------------------------------------
# Test
# ----------------------------------------------

from TestDecor import UtTest

def CoreTraceTestImpl():
    res = []
        
    def Fn(obj, text : str, textId: str):
        res.append(text)
        
    x = 5
        
    d = CoreTrace(Fn, True)
    d.Add(x, 'trace1')
    d.Enable(False)
    d.Add(x, 'trace2')
        
    assert len(res) == 1    

@UtTest
def CoreTraceTest():
    CoreTraceTestImpl()
 
# ----------------------------------------------

if __name__ == "__main__":
    
    import unittest
    unittest.main()