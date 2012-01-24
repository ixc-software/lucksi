
import os.path
import glob
import unittest as ut

# --------------------------------------------------------

def _PackageAllModules(packageName : str, excludePrivate : bool) -> (str):
        
    d = {}
    exec("import " + packageName, d)
    path = os.path.split(d[packageName].__file__)[0]
    
    path = os.path.join(path, "*.py")
    modules = glob.glob(path)
    
    res = []
    
    for i in modules:
        fileName = os.path.split(i)[1]
        if fileName.startswith('__init__.py'): continue        
        if fileName.startswith('_') and excludePrivate: continue
        n = fileName.rfind('.')
        res.append( fileName[:n] )
    
    return res 

# --------------------------------------------------------

def _SelectItemsFromModule(module : str, filterFn : lambda str, object: None) -> [object]:
    d = {}
    exec("from {0} import *".format(module), d)        
    return [d[i] for i in d if filterFn(i, d[i])]

# --------------------------------------------------------

def ImportAllFromPackage(packageName : str, into : dict):
    modules = _PackageAllModules(packageName, True)
    for m in modules:
        if m.startswith('_'): continue
        exec("from {0}.{1} import *".format(packageName, m), into)

# --------------------------------------------------------

def TestSuiteForModule(moduleName : str, runVerbocity = -1) -> ut.TestSuite:
            
    src = "import {0}\na = ut.TestLoader().loadTestsFromModule({0})\n".format(moduleName)        
    exec(src)
    
    res = eval("a")
    
    if runVerbocity >= 0:
        ut.TextTestRunner(verbosity = runVerbocity).run(res)
    
    return res

#def TestSuiteForModule(moduleName : str, runVerbocity = -1) -> ut.TestSuite:
#        
#    def fnFilter(name, obj):
#        return isinstance(obj, type) and issubclass(obj, ut.TestCase)
#    
#    sel = _SelectItemsFromModule(moduleName, fnFilter)
#        
#    res = ut.TestSuite()    
#    for f in sel:
#        res.addTest( f() )
#        
#    if runVerbocity >= 0:
#        ut.TextTestRunner(verbosity = runVerbocity).run(res)
#    
#    return res
    
# --------------------------------------------------------

def TestSuiteForPackage(packageName : str, runVerbocity = -1, 
                        ignorePrivateModules = False):
        
    modules = _PackageAllModules(packageName, False)

    res = ut.TestSuite()    
    
    for m in modules:
        if ignorePrivateModules and m.startswith("_"):
            continue
        res.addTest( TestSuiteForModule(packageName + "." + m) )
        
    if runVerbocity >= 0:
        ut.TextTestRunner(verbosity = runVerbocity).run(res)
            
    return res

# --------------------------------------------------------

if __name__ == "__main__":
    
    TestSuiteForModule("TestDecor", 2)
    TestSuiteForPackage("CDUtilsPack", 2)
        
