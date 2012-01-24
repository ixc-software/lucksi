'''
Created on May 14, 2010

@author: root
'''

from PackUtils.Misc import StrToInt, ConvertErr
from PackUtils.CoreBaseException import CoreBaseException

# --------------------------------------------------------------------

class ValidatorErr(CoreBaseException):
    def __init__(self, err : str, val, fieldName : str):
        Exception.__init__(self, "In widget '{2}': {0}: '{1}'".format(err, val, fieldName))

# --------------------------------------------------------------------

class ValidatorTypeErr(ValidatorErr):
    def __init__(self, typeDesc : str, val, fieldName : str):
        ValidatorErr.__init__(self, "Value type must be '{}'".format(typeDesc), 
                              val, fieldName)

# --------------------------------------------------------------------

def CheckValueType(val, type, fieldName):
    if isinstance(val, type): return 
    raise ValidatorTypeErr(type.__name__, val, fieldName)

# --------------------------------------------------------------------

class Validator:
    """
    Base validator for GuiConfigField values. Check always succeed.
    """
    def Check(self, val, fieldName : str):
        return True

# --------------------------------------------------------------------

class ValidatorInt:
    def __init__(self, min : int = None, max : int = None, allowNone = True):
        if min != None and max != None: assert min <= max
        self.min = min
        self.max = max
        self.allowNone = allowNone
        
    def Check(self, val : int, fieldName : str):
        if val == None and self.allowNone:
            return
        
        CheckValueType(val, int, fieldName)
        
        if self.min != None and val < self.min:
            raise ValidatorErr("Value < min", val, fieldName)
        if self.max != None and val > self.max:
            raise ValidatorErr("Value > max", val, fieldName)


# --------------------------------------------------------------------

class ValidatorPort(ValidatorInt):
    def __init__(self):
        ValidatorInt.__init__(self, 0, 65535)
        
# --------------------------------------------------------------------

def _IsValidHost(text : str, withPort : bool) -> bool:
    if not text : return True
    hostPort = text.split(':')
    hostNums = hostPort[0].split('.')
    if len(hostNums) != 4: return False
    for n in hostNums:
        try:
            StrToInt(n.strip())
        except ConvertErr:
            return False      
    if withPort:
        if len(hostPort) != 2: return False
        try: 
            StrToInt(hostPort[1].strip())
        except ConvertErr: 
            return False
    else:
        if len(hostPort) != 1: return False
        
    return True    
    
# --------------------------------------------------------------------

class ValidatorHost:
    def __init__(self, ip_port = True, ip = False, port = False):
        self.useIp = ip;
        self.usePort = port
        self.useBoth = ip_port
        
    def Check(self, val : str, fieldName : str):
        CheckValueType(val, str, fieldName)
        
        if not val: return
        
        val = val.strip()
        
        formats = []
        if self.useIp:
            if _IsValidHost(val, False): return
            formats.append("'0.0.0.0'")
            
        if self.usePort:
            try:
                valInt = StrToInt(val)
                p = ValidatorPort()
                p.Check(valInt, fieldName)
            except (ConvertErr, ValidatorErr):
                formats.append("'0'")
            else:
                return
            
        if self.useBoth:
            if _IsValidHost(val, True): return
            formats.append("'0.0.0.0:0'")
        
        err = "Invalid host format (should be: {})".format(" or ".join(formats))
        raise ValidatorErr(err, val, fieldName)

# --------------------------------------------------------------------

class ValidatorName:
    def __init__(self, allowEmpty : bool = False):
        self.allowEmpty = allowEmpty
        
    def Check(self, val : str, fieldName : str):
        CheckValueType(val, str, fieldName)
        
        def IsValidName(name : str) -> bool:
            if not name:
                if self.allowEmpty: return True 
                return False
            if not name[0].isalpha(): return False
            for ch in name:
                if ch.isalpha() or ch.isdigit() or ch == '_':
                    continue
                return False
            return True         
        
        if IsValidName(val): return
        err = "Invalid name format (allowed: A-Z, a-z, _, 0-9; must start with letter)"
        raise ValidatorErr(err, val, fieldName)

# --------------------------------------------------------------------

class ValidatorBool:
    def Check(self, val : bool, fieldName : str):
        CheckValueType(val, bool, fieldName)

# --------------------------------------------------------------------

def IsValidator(obj) -> bool:
    
    def Is(objType) -> bool:
        return isinstance(obj, objType)
    
    return (Is(Validator) or Is(ValidatorHost) or Is(ValidatorInt) or 
            Is(ValidatorName) or Is(ValidatorBool) or Is(ValidatorPort))

# --------------------------------------------------------------------
# Tests
# --------------------------------------------------------------------    

from TestDecor import UtTest
from AllTest.TestRunner import GTestRunner

@UtTest
def ValidatorsTest():
    
    def Valid(validator, valList):
        for val in valList:
            validator.Check(val, 'x')
    
    def Invalid(validator, valList):
        for val in valList:
            try:
                validator.Check(val, 'x')
            except (ValidatorErr, ConvertErr):
                continue
            assert 0 and "No test exception!"
    
    def ValidatorHostTest():
        v = ValidatorHost() # ip_port
        Valid(v, ["0.0.0.0:0", " 0 . 0 . 0 . 0 : 0 ", ""])
        Invalid(v, ["0.0.0.0", "0", "x.0.0.0:0"])
        
        v = ValidatorHost(ip_port = False, ip = True)
        Valid(v, ["0.0.0.0", " 0 . 0 . 0 . 0 ", ""])
        Invalid(v, ["0.0.0.0:0", "x.0.0.0" , "0"])
        
        v = ValidatorHost(ip_port = False, port = True)
        Valid(v, ["25", " 0 ", ""])
        Invalid(v, ["0.0.0.0:0", "0.0.0.0", "x.0.0.0"])
    
        v = ValidatorHost(ip_port = False, port = True, ip = True)
        Valid(v, ["0", "0.0.0.0", ""])
        Invalid(v, ["0.0.0.0:0"])
        
        v = ValidatorHost(ip_port = True, port = True, ip = True)
        Valid(v, ["0", "0.0.0.0", "0.0.0.0:0", ""])
        Invalid(v, ["x", "-5", "65536",  "0.0.0.0:x"])
        
        
    def ValidatorNameTest():
        v = ValidatorName(False)
        Valid(v, ["asdf", "a1s2df_"])
        Invalid(v, ["1asdf", "a sdf", "_asdf", ""])
        
        v = ValidatorName(True)
        Valid(v, [""])
        
    def ValidatorIntTest():
        v = ValidatorInt()
        Valid(v, [-1, 0, 5, 2000])
        
        v = ValidatorInt(min = 5)
        Valid(v, [5, 2000])
        Invalid(v, [-1, 0])
        
        v = ValidatorInt(max = 5)
        Valid(v, [-1, 5])
        Invalid(v, [2000])
        
        v = ValidatorInt(min = 0, max = 5)
        Valid(v, [0, 4, 5])
        Invalid(v, [-1, 2000])
        
        Invalid(v, "as")
    
    def ValidatorPortTest():
        p = ValidatorPort()
        Valid(p, [0, 50, 65535])
        Invalid(p, [-1, 65536, 70486])
    
    
    def Impl(outputFn):
        ValidatorHostTest()
        ValidatorNameTest()
        ValidatorIntTest()
        ValidatorPortTest()
        assert IsValidator(ValidatorInt())
        
        outputFn('ValidatorsTest: OK')
        
    GTestRunner.Run(Impl, traceTest = False)

# --------------------------------------------------------------------

if __name__ == "__main__":
    import unittest
    unittest.main()
    
    