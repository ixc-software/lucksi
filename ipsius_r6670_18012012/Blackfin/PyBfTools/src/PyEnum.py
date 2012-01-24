import inspect
import collections
import copy

# ---------------------------------------------

class EnumClassException(Exception):
    pass

# ---------------------------------------------

class EnumInfo:
    def __init__(self, d : collections.OrderedDict):
        isinstance(d, collections.OrderedDict)
        
        class EnumItem:
            def __init__(self, name, val, owner):
                self.__name = name
                self.__val = val
                self.__owner = owner
                
            @property
            def Name(self):
                return self.__name
                
            @property
            def Value(self):
                return self.__val
            
            @property
            def Owner(self):
                return self.__owner
            
            def __str__(self):
                return "{0}: {1}".format(self.Name, self.Value)
            
        def MakeDic(d):
            res = collections.OrderedDict()
            for k in d: res[k] = EnumItem(k, d[k], self)
            return res
        
        self.d = MakeDic(d)
        
    def __str__(self):
        return '; '.join( [str(self.d[k]) for k in self.d] )
    
    def __iter__(self):
        for k in self.d: yield k
        
    def __getitem__(self, key):
        return self.d[key]
    
    def __getattr__(self, name):
        if name in self.d: return self.d[name]
        raise AttributeError(name) 
    
    def __delattr__(self, name):
        raise EnumClassException('Enum is read-only!')
    
    def __setattr__(self, name, value): 
        if len(self.__dict__) == 0:
            return object.__setattr__(self, name, value)
        else:
            raise EnumClassException('Enum is read-only!')
                
    def ValueToItems(self, val : int) -> 'tuple of EnumItem':
        return tuple([self.d[k] for k in self.d if self.d[k].Value == val])
    
    def VerifyItem(self, item, throwException = True) -> bool:
        
        def ItemOk():
            if not hasattr(item, 'Owner'): return False            
            return item.Owner is self
        
        res = ItemOk()
        if not res and throwException: raise EnumClassException('VerifyItem fail!')         
        return res

# ---------------------------------------------

def EnumClassEx(f, context):

    CIdent = '    '
    CDict  = 'specialEnumDict'
        
    # result is (header line index, function name)
    def FindHeader(src : [str]) -> (int, str):
        for i, s in enumerate(src):
            s = s.strip()
            if s.startswith('def ') and s.endswith(':'):
                n = s.find('(')
                if n < 0: raise EnumClassException('Bad function header ' + s)
                return (i, s[3:n].strip())
        raise EnumClassException('Function header not found in ' + src)
    
    def MakeFields(fnSrc : [str]) -> [str]:
        src = []
        varList = []
        
        for s in fnSrc:
            s = s.strip()
            if (s == ''): continue
            if (s.startswith('#')): continue
            
            evalLine = s
            varName = ''
            
            n = s.find('=')
            if n >= 0:
                nComment = s.find('#', 0, n)
                if nComment >= 0: continue
                varName = s[:n].strip()            
            else:
                nComment = s.find('#')
                if nComment >= 0: s = s[:nComment].strip()
                varName = s
                evalLine = s + ' = '
                if len(varList) == 0: 
                    evalLine += '0'
                else:
                    evalLine += varList[-1] + ' + 1'
    
            if varName in varList: raise EnumClassException('Dublicate name ' + varName)
            if varName.startswith('__'): raise EnumClassException('Bad name ' + varName)
            varList.append(varName)
            
            src.append(CIdent + evalLine)
            src.append(CIdent + CDict + '["' + varName + '"] = ' + varName)
            
        return src
                                
    fnSrc = inspect.getsource(f).split('\n') 
    h = FindHeader(fnSrc)
    
    className = h[1] + '_specialEnumClass' 

    src = []
    src.append('import collections')
    src.append('class ' + className + ':')
    src.append(CIdent + CDict + ' = collections.OrderedDict()') 
    
    src.extend( MakeFields( fnSrc[h[0] + 1:] ) )
            
    exec('\n'.join(src), context)               # create 'className'    
    d = eval(className + '.' + CDict, context)  # get dict
    exec('del ' + className, context)           #del 'className'       

    return EnumInfo(d) 

# ---------------------------------------------

# decorator: function -> class
def EnumClass(f):
    
    def SumContext(globs, locs : dict) -> dict:
        g = copy.copy(globs)
        if locs is not None:
            for k in locs: g[k] = locs[k]
        return g
    
    # get frame of called function
    frame = inspect.currentframe().f_back
    assert frame is not None
    
    try:
        functionIsGlobal = frame.f_back is None
        context = SumContext(frame.f_globals, None if functionIsGlobal else frame.f_locals) 
    finally:
        del frame
    
    return EnumClassEx(f, context)

