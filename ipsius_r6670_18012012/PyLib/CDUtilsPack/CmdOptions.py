
import collections
import unittest
from collections import Iterable
from TestDecor import UtTest

from CDUtilsPack.StrUtils import AlignStr
from CDUtilsPack.MiscUtils import IsMain
from CDUtilsPack.MetaUtils import UserException 

# ----------------------------------------------

class CmdOptions:
    
    """ 
    Command line parser
     
    Known limitations: 
        - can't use one options multiple times in command line
                
    """
    
    CParamsDict = {
        'str': lambda s: s,
        'int': lambda s: int(s),
    }
    
    class Error(UserException):
        pass
    
    class __ResultDict(collections.OrderedDict):
        
        def __init__(self, correctKeys : [str], allowAddNewKeys : bool):
            
            super().__init__()
            
            self.correctKeys = correctKeys
            self.allowAddNewKeys = allowAddNewKeys
            
        def __setitem__(self, key, value):
            
            self.__VerifyKey(key)         
            super().__setitem__(key, value)
            
        def __contains__(self, item):
            
            self.__VerifyKey(item)                             
            return super().__contains__(item)
        
        def __VerifyKey(self, key):
                        
            if self.allowAddNewKeys: return
             
            if not key in self.correctKeys: 
                raise Exception('Bad key {0} allowed {1}'.format(key, self.correctKeys))
    
    class __Params:
        
        class Item:
            
            """ Parse 'name:type*' """                
            def __init__(self, s):
                
                assert len(s) > 0

                # optional
                self.optional = False
                if s[-1:] == "*":                         
                    self.optional = True
                    s = s[:-1]
                
                # type/name
                type = "str"                    
                n = s.find(":")
                if n < 0: 
                    name = s
                else:
                    name = s[:n] 
                    type = s[n+1:]
                    
                if type not in CmdOptions.CParamsDict:
                    raise CmdOptions.Error("Bad param type -- " + type)
                    
                self.name = name
                self.type = type
                self.typeFn = CmdOptions.CParamsDict[type]
                
            @property
            def Must(self): return not self.optional
            
        def __init__(self, params : [str]):
                                                    
            self.items = []
            self.unlimited = False
            self.hasOptional = False            
            self.mustCount = 0
            
            for index, s in enumerate(params):
                
                # wildcard
                if s == '*':
                    if index + 1 != len(params): raise CmdOptions.Error("'*' not last!")
                    if self.hasOptional: raise CmdOptions.Error("'*' + optional param(s) is meanless!")                        
                    self.unlimited = True
                    break
                
                # create
                i = self.Item(s)
                self.items.append(i)
                
                # mustCount
                if i.Must: 
                    self.mustCount += 1
                else: 
                    self.hasOptional = True 
                
                # verify: Must <- Must
                if index > 0:
                    prev = self.items[index - 1]
                    
                    if i.Must: assert prev.Must
                     
        def MaxResultCount(self) -> int:
            
            if not self.unlimited:             
                if len(self.items) == 0: return 0            
                if self.mustCount == 1 and len(self.items) == 1:  return 1
                
            return 2
                                    
        """ yield (convertFn, typeName, isMust) """
        def __iter__(self):
            
            for i in self.items:
                yield (i.typeFn, i.type, i.Must)
                
            if self.unlimited:
                
                fn = CmdOptions.CParamsDict['str']
                while True:
                    yield (fn, 'str', False)
                                            
        def AsString(self) -> str:
            
            s = ''
            indexOpened = False

            for index, i in enumerate(self.items):
                
                if index == self.mustCount:
                    indexOpened = True
                    s += '['
                    
                s += i.name
                if index + 1 < len(self.items) or self.unlimited: 
                    s += ' '
                                                
            # add "*"
            if self.unlimited:
                if not indexOpened:
                    indexOpened = True
                    s += '['
                s += '...'
            
            # close
            if indexOpened:
                s += ']'
            
            return s
                
    class __Option:
        
        """ For class fields look in Add() """
        
        @property
        def Name(self) -> str: return self.names[0]
                
        def CmdNotation(self) -> str:
            s = '/' + self.Name
            elipsis = '<>' if self.req else '[]'
            
            params = self.params.AsString()
            if params: s += ' ' + params
                                        
            return elipsis[0] + s + elipsis[1]
        
        def HelpNotation(self) -> str:
            s = '; '.join(['/' + opt for opt in self.names])
            
            params = self.params.AsString()
            if params: s += ' ' + params
                                
            return s
        
    # autoAddShortNames -> in Add() try generate short option name, if names is str, not tuple 
    def __init__(self, autoAddShortNames : bool = False, addHelp : bool = True):
        
        self.options = []
        self.autoAddShortNames = autoAddShortNames
        self.readOnly = False
        
        if addHelp: 
            self.Add(('help', 'h'), 'This help')
                
    def __Raise(self, msg):
        raise self.Error(msg)
                        
    def __FindOption(self, name : str) -> None or __Option:
        for o in self.options:
            if name in o.names: return o
        return None

    # params encoded as tuple of "name[:type]", type from CParamsDict; example: "name:str" or "name" 
    def Add(self, names : str or (str), desc : str = '', params : (str) = (), req : bool = False):
        
        assert not self.readOnly
        assert not isinstance(params, str)  # protection from bug params = ('a')
                
        # fix names
        if isinstance(names, str):
            name = names
            names = [name, ]
            
            if self.autoAddShortNames:
                for i in range(len(name)):
                    s = name[:1 + i]
                    if self.__FindOption(s) is None:
                        names.append(s)
                        break 
       
        # verify
        for name in names: 
            if self.__FindOption(name) is not None:
                self.__Raise("Dublicate option name " + name)
                                    
        assert isinstance(names, Iterable)
        assert isinstance(names[0], str) and len(names[0]) > 0  
                
        # add                
        opt = self.__Option()
        opt.names = names
        opt.desc = desc
        opt.params = self.__Params(params) 
        opt.req = req
        
        self.options.append(opt)
        
    def __EnterReadOnly(self):
        
        if self.readOnly: return
        
        # action on enter reas-only state
        # ... 
                        
        self.readOnly = True
        
        
    def HelpText(self) -> str:
        
        self.__EnterReadOnly()
        
        s = 'Usage: script '
        
        for opt in self.options:
            s += opt.CmdNotation() + ' '
        s += '\n'
        
        notationWidth = max( [ len(opt.HelpNotation()) for opt in self.options ] )
                
        for opt in self.options:
            s += "    {0}  {1}\n".format( AlignStr(opt.HelpNotation(), -notationWidth), opt.desc)
        
        return s
    
    def __ArgsToDict(self, args : (str), allowAddNewKeys : bool) -> {}:
        
        correctNames = [o.Name for o in self.options]
        result = self.__ResultDict(correctNames, allowAddNewKeys)
        currIndx = 0
        paramChar = None
        
        while currIndx < len(args):
            
            optName = args[currIndx]

            # prefix check
            if paramChar:
                if optName[0] != paramChar:
                    self.__Raise("Unexpected option perfix in " + optName)                    
            else:
                if not(optName.startswith("/") or optName.startswith("-")):
                    self.__Raise("No option prefix in " + optName)
                paramChar = optName[0]

            # verify option
            opt = self.__FindOption(optName[1:])
            if opt is None: self.__Raise("Unknown option " + optName)
            
            optName = opt.Name
            if optName in result: self.__Raise("Dublicate option " + optName)
            
            currIndx += 1
                        
            # collect params
            params = []
            for pFn, pType, pMust in opt.params:
                
                if not pMust:
                    if currIndx >= len(args): break
                    if args[currIndx].startswith(paramChar): break
                
                if currIndx >= len(args): self.__Raise("No extra params for option " + optName)
                
                optS = args[currIndx]
                try:
                    params.append( pFn(optS) )
                except Exception as e:
                    self.__Raise("Can't convert {0} as {1} with error: {2}".format(optS, pType, e))
                    
                currIndx += 1
                
            # fix params
            count = opt.params.MaxResultCount()
            
            if count == 0:
                params = None
            elif count == 1:
                assert len(params) == 1
                params = params[0]
            else:
                params = tuple(params)
                
            # debug
#            print(optName + ' = ' + str(params))
            
            # add to result
            result[optName] = params
            
        return result
            
    def Parse(self, args : (str), exitOnHelp = True, allowAddNewKeys = False) -> {}:
        
        self.__EnterReadOnly()

        result = self.__ArgsToDict(args, allowAddNewKeys)
                                        
        if 'help' in result: 
            # process 'help'                                                
            if len(result) != 1:
                self.__Raise('Extra options with "help"')            
            if exitOnHelp:
                print(self.HelpText())
                exit()                                
        else:   
            # process req params                    
            for opt in self.options: 
                if opt.req:
                    if opt.Name not in result:
                        self.__Raise('No required option ' + opt.Name)
                        
        return result        

# --------------------------------------------

@UtTest
def Test(silence : bool = True):
    
    from CDUtilsPack.MiscUtils import AssertForException
        
    opts = CmdOptions(autoAddShortNames = True)
    
    opts.Add( 'com',              'COM port number', ("number:int",), True)
    opts.Add( ('file', 'f'),      'Input file',      ('fileName', 'fileMode:str') )
    opts.Add( 'fast',             'Fast write mode' )
    opts.Add( 'verbose',          'Enable verbose mode')

    # help text
    help = opts.HelpText() 
    if not silence: print(help)
    
    # call help
    r = opts.Parse( ("-h",), exitOnHelp = False )
    assert r == { "help" : None }
    
    # call for params
    r = opts.Parse( ("/com", "10", "/f", "a", "b", "/v"), exitOnHelp = False )
    assert r == {"com" : 10, 
                 "file" : ("a", "b"),
                 "verbose" : None}
    
    # work with result
    assert "com" in r
    assert "fast" not in r
    
    def f(): v = "_" not in r  # can't test for bad key
    AssertForException(f)
        
    assert r["com"] == 10

    r["com"] = 11
    assert r["com"] == 11
    
    def f(): r["bad_key"] = None  # can't add bad key
    AssertForException(f)            

# --------------------------------------------

@UtTest
def Test2(silence : bool = True):
    
    from CDUtilsPack.MiscUtils import AssertForException
    
    opts = CmdOptions(autoAddShortNames = False)
    
    opts.Add( 'void',              'Void param', req = True)
    opts.Add( 'm1',                'Must 1',                ('must0',) )
    opts.Add( 'm2',                'Must 2',                ('must0', 'must1') )
    opts.Add( 'm2v2',              'Must 2 var 2',          ('must0', 'must1', 'var0:int*', 'var1:int*') )
    opts.Add( 'any',               'Any',                   ('*',) )
    opts.Add( 'm1any',             'Must 1 + any',          ('must0', '*') )
    
    # meanless
#    opts.Add( 'v2any',             'Var 2 + any',           ('var0*', 'var1*', '*')  )
#    opts.Add( 'm1v1any',           'Must 1 var 1 any',      ('m0', 'v0*', '*') )
    
    # help text
    help = opts.HelpText() 
    if not silence: print(help)
    
    # test
    CKeys = '/void /m1 a /m2 a b /m2v2 a b 7 /any a b c d e /m1any a b c'
    r = opts.Parse( CKeys.split(' '), exitOnHelp = False )
#    if not silence: print(r)

    refD = { 
            'void':None,
            'm1':'a',
            'm2':('a', 'b'),
            'm2v2':('a', 'b', 7),
            'any':('a', 'b', 'c', 'd', 'e'),
            'm1any':('a', 'b', 'c')
    }
    
    assert r == refD
    

# --------------------------------------------

def _ExtraParamTest():

    opts = CmdOptions(autoAddShortNames = False)
    
    opts.Add( 'void', 'Void param', req = True)
    
    CKeys = '/void abc'
    r = opts.Parse( CKeys.split(' '), exitOnHelp = False )

    print(r)    
    
    
    
# --------------------------------------------

if IsMain():

#    Test.Call(False)
#    Test2.Call(False)

#    _ExtraParamTest()
    
    unittest.main()    


