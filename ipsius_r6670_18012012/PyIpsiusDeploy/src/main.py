'''
Created on 22.12.2009

@author: Alex
'''

import os.path
import os
import sys
import shutil
import datetime
import time
import glob

import CDUtils
from CDUtils import UserException

from IpsiusInstallDesc import DescriptionType
from IpsiusInstallDesc import Version 

from InstallDesc import InsertEnvVarsInPath # todo remove

# -------------------------------------------

GDescType = DescriptionType()


# -------------------------------------------

def Install(srcRoot, dstRoot : str, ver : Version, tags : [str], 
            archiveDir : str, outputClean : bool):       
    
    print("Install, root folder {0}, result {1}".format(srcRoot, dstRoot))
#    print( CDUtils.CallInfo() )    

    def CleanOutput():
        
        if not os.path.exists(dstRoot): return
        
        print('Removing...')
        shutil.rmtree(dstRoot, ignore_errors = True)
     
    desc = GDescType.GetInstallDesc(tags, ver)
               
    # remove dst folder
    CleanOutput()
        
    # process all items
    print('Copying...')
    desc.Install(srcRoot, dstRoot, tags)
        
    # archive
    if archiveDir is not None:
        GDescType.MakeArchive(archiveDir, dstRoot, tags)
        
        if outputClean:
            CleanOutput()   
             
    # done
    print('Done!')

# -------------------------------------------

def Main(cmd : [str]):              
    
    def ParseOptions() -> {}:
        
        opts = CDUtils.CmdOptions(autoAddShortNames = True)
        
        opts.Add( 'b_log',  'Build log')
        opts.Add( 'b_main', 'Build Ipsius')
                
        opts.Add( 'source', 'Source root directory', ('dir',) )
        opts.Add( 'output', 'Output directory',      ('dir',) )
                
        opts.Add( 'pack',   'Make archive',          ('archive*',) )  
        opts.Add( 'clean',  'Delete output directory after archiving' )
                
        opts.Add( 'tags',   'Select tags (from {0})'.format(GDescType.CorrectTags().Info()),   ('*',) )
        
        opts.Add( 'pub_ver', 'Public version', ("pubVer", ) )
        
        #opts.Add( '' )
                
        return opts.Parse(cmd)
    
    
    opts = ParseOptions()
            
    # process options    
    scpDir = CDUtils.GetScriptDir()
    
    if 'source' not in opts:
        opts['source'] = CDUtils.DirUp(scpDir, 2)
        
    ver = Version( opts.get('pub_ver', '') )
    
    if 'output' not in opts:               
        opts['output'] = os.path.join( CDUtils.DirUp(scpDir, 1), ver.FileName() )
        
    if 'tags' in opts:
        GDescType.CorrectTags().VerifySet(opts['tags'])        
    else:
        opts['tags'] = GDescType.CorrectTags().All()

    if 'pack' not in opts:
        opts['pack'] = None
    else:
        v = opts['pack']
        opts['pack'] = v[0] if v else ""
        if opts['pack'].startswith("'") and opts['pack'].startswith("'"):
            opts['pack'] = opts['pack'][1:-1]            
    
    outputClean = False
    if 'clean' in opts:
        outputClean = True
        if 'pack' not in opts: 
            raise UserException("Arguments error: 'clean' without 'pack'")            
    
    # fix tags 
    tags = GDescType.CorrectTags().FixTagsForDebug( opts['tags'] )    
    
    # process build options    
    if 'b_log' in opts:  GDescType.BuildLog(tags)                    
    if 'b_main' in opts: GDescType.BuildIpsius(tags) 
    GDescType.BuildLang(tags)
        
    # do install
    Install(opts['source'], opts['output'], ver, tags, opts['pack'], outputClean)

    
# -------------------------------------------

"""
        
    - auto make fwu? BfMainE1 auto build - ?
    
    * options engine improve
    
    - linux support - ?!
    - "pack"  -> setup.exe
    - add dir -> "pack" support
    

"""

# -------------------------------------------

def BatExecTest():

    scpDir = CDUtils.GetScriptDir()
    
    batName = os.path.join(scpDir, r'..\..\ProjUdpLogViewer\_build_release_auto.bat')
    batName = os.path.abspath(batName)
    
    print(batName)
    
    
    currDir = os.getcwd()  # store cwd
    
    os.chdir( os.path.split(batName)[0] )
    
    cmd = batName
    res = os.system('"' + cmd + '"')
    print(res)
    
    os.chdir(currDir)  # restore cwd
    
    exit()

# -------------------------------------------

def StampTest():
    
    t = time.time()
    
    print( time.gmtime(1.0) )
    print( time.localtime(t) )
    print( CDUtils.DateTimeStamp(t) )
    
    exit()

# -------------------------------------------

if __name__ == '__main__':
    
#    BatExecTest()    
#    DatetimeTest()
#    StampTest()

    Main( sys.argv[1:] )
        
