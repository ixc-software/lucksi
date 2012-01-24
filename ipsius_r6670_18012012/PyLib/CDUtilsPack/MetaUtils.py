
import copy
import unittest

from types import FunctionType
from TestDecor import UtTest

# ---------------------------------------------------

def ListClassFields(obj, ignores : [str] = []) -> [str]:
    fields = dir(obj)
    
    # remove __xx__ items
    fields = [s for s in fields if not(s.startswith('__') and s.startswith('__'))]
    
    # remove callable (with __call__)
    fields = [s for s in fields if not( hasattr(getattr(obj, s), '__call__') )]
        
    def DictToStr(dict) -> str:
        return "{ " + ", ".join([str(k) + ": " + str(dict[k]) for k in dict]) + " }" 
    
    res = []
    for f in fields:
        
        if f in ignores: continue
        
        attr = getattr(obj, f)
        
        s = ''
        if issubclass(type(attr), dict): s = DictToStr(attr)
        else: s = str(attr)
            
        res.append(f + ": " + s)
    
    return res

# -----------------------------------------------------     

@UtTest
def Test_ListClassFields(verbose : bool = False):
    
    class A:
        a = 10
        b = 1.5
        c = ["a", "b"]
        
    s = ListClassFields(A())
    
    if verbose:
        print("\n".join(s))
    
# -----------------------------------------------------     

'''
    Add all compare methods to class
    
    Example:
            
    class X:
            
        def cmp(self, other) -> int:
            if self > other: return 1
            if self < other: return -1
            return 0
                
    AddClassCompare(X)  # or AddClassCompare(X, X.cmp)
                
'''
def AddClassCompare(obj : type, cmpFn : FunctionType = None) -> None: 
    
    assert isinstance(obj, type)
    cmpFn = cmpFn or obj.cmp
    
    def GetFn(cmpInPlace):
        
        def Fn(self, other):
            x = cmpFn(self, other)
            return cmpInPlace(x)
        
        return Fn 
                           
    obj.__lt__ = GetFn(lambda x: x < 0)
    obj.__le__ = GetFn(lambda x: x <= 0)
    obj.__eq__ = GetFn(lambda x: x == 0)
    obj.__ne__ = GetFn(lambda x: x != 0)
    obj.__gt__ = GetFn(lambda x: x > 0)
    obj.__ge__ = GetFn(lambda x: x >= 0)    

# compare decorator        
def ComparableClass(cmpFn : FunctionType = None) -> FunctionType:
    
    def fn(obj : type) -> type:
        assert isinstance(obj, type)        
        objCopy = copy.copy(obj) # is that make sense? 
        AddClassCompare(objCopy, cmpFn)
        return objCopy
    
    if cmpFn: assert isinstance(cmpFn, FunctionType)
    
    return fn

# -----------------------------------------------------     

@UtTest
def Test_ComparableClass():

    class A: 
        def __init__(self, val = 0):
            self.val = val
            
        def Compare(self, other):
            if self.val > other.val: return 1
            if self.val < other.val: return -1
            return 0

    @ComparableClass()  # don't forget add "()"
    class B(A):
        def __init__(self, val = 0):
            A.__init__(self, val)
    
        def cmp(self, other):
            return A.Compare(self, other)
        
    @ComparableClass(lambda x, y: x.Compare(y))  # must use lambda, not direct C.Compare
    class C(A):
        def __init__(self, val = 0):
            A.__init__(self, val)
        
    def TestType(T : type):
        assert T(10) > T(3)
        assert T(3) <= T(3)
        assert not T(7) == T(3)
        
    TestType(B)
    TestType(C)


# -----------------------------------------------------     

def CreateLazyInitProxy(fnCreate : FunctionType, traceFn : FunctionType = None):
    
    instance = None
    
    def LazyInit() -> object:
        nonlocal instance        
        if instance is None:
            if traceFn: traceFn('Trace: Instance created!')          
            obj = fnCreate()
            assert obj is not None
            instance = obj

        return instance
                        
    class Proxy:
        def __init__(self):
            pass
                                                
        def __getattribute__(self, name):
            i = LazyInit()
            if traceFn: traceFn('Trace: Access get {0}'.format(name))         
            return i.__getattribute__(name)
        
        def __setattr__(self, name, value):
            i = LazyInit()
            if traceFn: traceFn('Trace: Access set {0}'.format(name))                     
            return i.__setattr__(name, value)
                    
    assert isinstance(fnCreate, FunctionType)
    
    return Proxy() 

# ------------------------------------     

@UtTest
def Test_CreateLazy(traceFn = lambda s: None):
    
    class A:
        def __init__(self):
            self.val = 0
            
        def Add(self, i):
            self.val += i

    traceFn('Create proxy')            
    a = CreateLazyInitProxy(lambda: A(), traceFn = traceFn)
    
    traceFn('Access to object')
    a.Add(1)   # get attr test
    assert a.val == 1
    
    a.val = 10  # set attr test
    assert a.val == 10
    
    assert isinstance(a, A)
    objDir = dir(a)
    assert "Add" in objDir 
    assert "val" in objDir
    

# -----------------------------------------------------  

class UserException(Exception):
    
    """ Exception with user-frendly message 
        Program can print it without stack trace info """
    
    pass

# -----------------------------------------------------  

def FnDoc(s : str):
    
    """ Decorator, set __doc__ attribute for function """
    
    def f(obj):
        assert isinstance(obj, FunctionType)
        obj.__doc__ = s
        return obj
    
    return f

@UtTest
def Test_FnDoc():
    s = "Doc string..."
    
    @FnDoc(s)
    def f():
        pass
    
    assert f.__doc__ == s

# -----------------------------------------------------

class PropertyException(Exception):
    pass

class PropertyDesc:
    
    def __init__(self, name, writeFn):
        self.name = name
        self.writeFn = writeFn
    
    def __get__(self, instance, owner):
        if not instance:
            return self 
        return getattr(instance, self.name)  
    
    def __set__(self, instance, value):
        if not self.writeFn: 
            raise PropertyException("Property '{0}' is read-only".format(self.name))
        self.writeFn(instance, value)
    
    def __delete__(self, instance):
        raise PropertyException("Can't delete property '{0}'".format(self.name))
    
def TestAllProperty(instance, silence = True):
    
    """ Try to access all Property() in instance """
    
    c = type(instance)
        
    for k, v in c.__dict__.items():
        if not isinstance(v, PropertyDesc): continue
        dummi = getattr(instance, k)
        if not silence:
            print(k, "=", dummi)
        
def Property( name : str, writable = False):

    """ writable must be 'True' or lambda instance, value: None """
            
    writeFn = None
    
    if isinstance(writable, bool):
        if writable:
            writeFn = lambda instance, value: setattr(instance, name, value)        
    else:
        assert isinstance(writable, FunctionType)
        writeFn = writable
                                
    return PropertyDesc(name, writeFn)

@UtTest
def Test_Property(tc):
            
    class A:
        
        def __init__(self):
            self.readonly = 10
            self.writable = 20
            self.customWritable = 30 
            
        def __SetCustomWritable(self, value):
            self.customWritable = value * 2
            
        ReadOnly = Property("readonly")
        Writable = Property("writable", True)
        CustomWritable = Property("customWritable", __SetCustomWritable)

    _eq = tc.assertEqual
    _ar = lambda: tc.assertRaises(PropertyException)
                        
    a = A()
        
    _eq(a.ReadOnly, 10)
    with _ar(): a.ReadOnly = 15
    with _ar(): del a.ReadOnly

    a.Writable = a.Writable + 1
    _eq(a.Writable, 21)
    
    a.CustomWritable = 100
    _eq(a.CustomWritable, 100 * 2)
    
    TestAllProperty(a)
    
    pass
    
# -----------------------------------------------------
  
if __name__ == "__main__":

    unittest.main()
        
    pass

