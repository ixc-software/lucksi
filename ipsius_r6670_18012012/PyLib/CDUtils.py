'''
Created on 10.11.2009

@author: Alex
'''

import ModulesMng
import unittest

ModulesMng.ImportAllFromPackage("CDUtilsPack", globals())

# -----------------------------------------------------  

#from CDUtilsPack.CmdOptions import *
#from CDUtilsPack.MetaUtils import *
#from CDUtilsPack.MiscUtils import *
#from CDUtilsPack.StrUtils import *
                       
# -----------------------------------------------------  
   
if IsMain():

#    unittest.main()
    
    ModulesMng.TestSuiteForPackage("CDUtilsPack", 2)

    # direct
#    import CDUtilsPack.MiscUtils
#
#    ts = unittest.TestLoader().loadTestsFromModule(CDUtilsPack.MiscUtils)
#    unittest.TextTestRunner(verbosity = 2).run(ts)

    pass