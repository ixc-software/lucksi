
import unittest

import CDUtilsPack.PyEnum
from TestDecor import UtTest     

# ------------------------------------------------------

_CYellowOffs = 1024

_prevGlobals = globals()
    
@CDUtilsPack.PyEnum.EnumClass
def _Color():
    clNone
    
    clRed   = 0x001    
    clGreen = 0x010     
    clBlue  = 0x100
    
    clYellow = _CYellowOffs + clRed + clGreen  # calc it, capture extern value
    clYellowNext                              # clYellow + 1
    clRedClone = clRed

# verify what PyEnum.EnumClass don't change globals()     
assert globals() == _prevGlobals
del _prevGlobals

@UtTest
def EnumTest(silentMode : bool = True):
    
    """ test global enum """
        
    # suppress print
    if silentMode: print = lambda *args: None
    else: print = __builtins__.print

    # read-only verify    
    def AssertForException(fn, exceptionType):
        wasException = False
        try:
            fn()
        except exceptionType:
            wasException = True
        assert(wasException)
    
    def f(): _Color.clGreen = 10
    AssertForException(f, CDUtilsPack.PyEnum.EnumClassException)
    def f(): del _Color.clRed
    AssertForException(f, CDUtilsPack.PyEnum.EnumClassException)

    # values verify
    assert _Color.clGreen.Value == 0x010
    assert _Color.clRed.Value == 0x001
    assert _Color.clYellowNext.Value == _Color.clYellow.Value + 1

    # print test
    print(_Color.clGreen)    
    print(_Color)
    
    for k in _Color:
        print(_Color[k].Name, '=', _Color[k].Value)  # or _Color[k]
    print()

    # test ValueToItems()
    def ValueToItemsTest(val : int, correctResult):
        
        def EnumItemsToStr(items):
            return '(' + ', '.join([str(i) for i in items]) + ')'
        
        assert correctResult == _Color.ValueToItems(val)
        
        print(val, '->', EnumItemsToStr(correctResult))
    
    ValueToItemsTest(1, (_Color.clRed, _Color.clRedClone))
    ValueToItemsTest(-1, tuple())
    ValueToItemsTest(0x10, (_Color.clGreen,))

    # typesafety test
    assert _Color.VerifyItem(_Color.clGreen)
    assert not _Color.VerifyItem('15', False)
    
    print('')

# -------------------------------------------------------------

@UtTest    
def EnumTest2(silentMode : bool = True):
    
    """ test local enum """    

    if silentMode: print = lambda *args: None
    else: print = __builtins__.print
    
    v = 1
        
    @CDUtilsPack.PyEnum.EnumClass        
    def x():
        one = v
        two = v + 10
        three
                
    print(x)
    print(x.one)
    
    
# --------------------------------------------------------------

if __name__ == "__main__":
    
    unittest.main()
    
    pass

    
    
