
import unittest
import types
import inspect

# -----------------------------------------

# decorator -- make unittest.TestCase from function fn 
def UtTest(fn) -> unittest.TestCase:
    
    # detect functions "def f(ts)" -- it's get self as ts param 
    def CallThisSelf(fn) -> bool:

        assert isinstance(fn, types.FunctionType)                
        info = inspect.getfullargspec(fn)

        if info.args == ['tc']: 
            return True

        return False
            
    class TestCase(unittest.TestCase):
                                                    
        def runTest(self):
            if callWithSelf: fn(self)
            else:            fn()
                        
        # call original function
        @staticmethod
        def Call(*arg, **argK):
            fn(*arg, **argK)
            
        def __str__(self):
            return fn.__module__ + '.' + fn.__name__
                        
    callWithSelf = CallThisSelf(fn)            
                                        
    return TestCase

# -----------------------------------------

@UtTest
def fn(silence : bool = True):
    if not silence:
        print("Ok!")

@UtTest
def fn2(tc):
    tc.assert_(1 > 0)

# -----------------------------------------

if __name__ == "__main__":

    # .Call() test
    class TcDummi:
        def assert_(self, val):
            assert val

    fn.Call(False)
    fn.Call(silence = False)    
    fn2.Call( TcDummi() )

    # run all tests
    import TestDecor
    ts = unittest.TestLoader().loadTestsFromModule(TestDecor)
    unittest.TextTestRunner(verbosity=2).run(ts)

    # run all tests - 2    
    unittest.main()
    
