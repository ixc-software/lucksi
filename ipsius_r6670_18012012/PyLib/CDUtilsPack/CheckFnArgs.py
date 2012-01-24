
def CheckFnArgs(fn, paramTypesTuple, returnType = None):
    """
    Checking function (or mrthod) 'fn' annotation and argument count. 
    """
    from inspect import getfullargspec
    
    assert fn
    spec = getfullargspec(fn)
    
    def GetNames() -> (str):
        res = spec[0]
        # ignore self 
        if len(res) > 0 and res[0] == 'self': res.remove(res[0])
        return res
    
    def GetNeedCount() -> int:
        res = -1
        try:
            res = len(paramTypesTuple)
        except TypeError as e:
            if (str(e).find("has no len()")):
                msg = "For functions with one parameter: paramTypesTuple == (type,)"
                raise AssertionError(msg) 
        return res
    
    types = spec[6]
    paramNames = GetNames()
    needParamCount = GetNeedCount()
    
    if len(paramNames) != needParamCount:
        msg = "Invalid number of parameters:\nhave: {0},\nneed: {1}".\
              format(types, paramTypesTuple)
        raise AssertionError(msg)

    for i, paramName in enumerate(paramNames):
        if paramName in types:
            if types[paramName] != paramTypesTuple[i]:
                msg = "Expected parameter type: '{0}', not '{1}'".\
                      format(paramTypesTuple[i].__name__,
                             types[paramName].__name__)
                raise AssertionError(msg)
            
    
    if returnType == None: return
    assert types['return'] == returnType
    
# --------------------------------------------------------------------
# Tests
# --------------------------------------------------------------------

from TestDecor import UtTest

def CheckFnArgsTestImpl():
    def Fn(data : str, i : int, j : int): pass
    CheckFnArgs(Fn, (str, int, int), None)
        
    def Fn2() -> int: pass
    CheckFnArgs(Fn2, (), int)
       
    def Fn3(i : int): pass
    CheckFnArgs(Fn3, (int,), None)
        
    def Fn4(obj, i : int): pass
    CheckFnArgs(Fn4, (None, int,), None)
       
    class FClass: 
        def Fn(self, i : int): pass
         
    CheckFnArgs(FClass().Fn, (int,), None)
    

@UtTest
def CheckFnArgsTest():       
    CheckFnArgsTestImpl()