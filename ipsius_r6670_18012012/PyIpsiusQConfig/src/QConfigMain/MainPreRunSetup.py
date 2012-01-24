'''
Created on Jul 13, 2010

@author: root
'''


def PreRunSetup(withUi : bool):
    
    # add ./src to PYTHONPATH
    import sys
    from CDUtilsPack.MiscUtils import DirUp, GetScriptDir
    sys.path.append( DirUp(GetScriptDir(), 1) )
    
    # make tmp folders
    from PackUtils.CorePathes import TempFiles    
    TempFiles.MakeDirs()
    
    # generate files
    if withUi:
        from PyUiGenerator import Generate  
        Generate(DirUp(GetScriptDir(), 1), TempFiles.DirUi)