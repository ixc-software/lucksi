'''
Created on May 17, 2010

@author: root
'''

from PackUtils.Misc import IntToStr, StrToInt, ConvertErr
from PackUtils.CoreBaseException import CoreBaseException


class ConverterErr(CoreBaseException):
    def __init__(self, fieldName : str, err : str):
        Exception.__init__(self, "{0} >> {1}".format(fieldName, err))

# -------------------------------------------------

class Converter:
    """
    Base converter for GuiDeviceConfigField. Using to convert field value to 
    widget value type and backward. Convert nothing.
    """
    def FromWidgetType(self, widgetName : str, val):
        return val
    
    def ToWidgetType(self, widgetName : str, val):
        return val
    
# -------------------------------------------------

class StrIntConverter:
    """
    Converter for GuiDeviceConfigField. Convert 'int' fields to 'str' 
    widget type and backward.
    """
    def FromWidgetType(self, widgetName : str, val : str):
        if not val: return None
        
        assert isinstance(val, str)
        try:
            return StrToInt(val)
        except ConvertErr as e:
            raise ConverterErr(widgetName, str(e))
    
    def ToWidgetType(self, widgetName : str, val : int):
        if val == None: return ''
        
        if not isinstance(val, int):
            err = "{0}: Expected value of type int: '{1}'".format(widgetName, val)
            raise AssertionError(err)
        return IntToStr(val)

# -------------------------------------------------

def IsConverter(obj) -> bool:
    return isinstance(obj, Converter) or isinstance(obj, StrIntConverter)

# -------------------------------------------------
# Tests
# -------------------------------------------------

from TestDecor import UtTest
from AllTest.TestRunner import GTestRunner


@UtTest
def ConvertersTest():
    def Impl(outputFn):
        c = Converter()
        assert c.FromWidgetType('w', 'x') == 'x'
        assert c.ToWidgetType('w', 'x') == 'x'
        
        c = StrIntConverter()
        assert c.FromWidgetType('w', '5') == 5
        assert c.ToWidgetType('w', 5) == '5'
        
        assert c.FromWidgetType('w', '') == None
        assert c.ToWidgetType('w', None) == ''
        
        outputFn('ConvertersTest: OK')
    
    GTestRunner.Run(Impl, traceTest = False)    

# -------------------------------------------------

if __name__ == "__main__":
    import unittest
    unittest.main()
    