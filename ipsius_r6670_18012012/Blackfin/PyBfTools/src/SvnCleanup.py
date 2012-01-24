'''
Created on 04.03.2010

@author: Alex
'''

import os.path
import sys
import tempfile
import shutil
import stat

from CDUtils import UserException

# --------------------------------------------

def OsExec(cmd, printCmd = False) -> int:
    if printCmd: print(cmd)
    res = os.system('"' + cmd + '"')
    if printCmd: print('')
    return res

# --------------------------------------------

def Main(args : [str]):
    
    psplit = os.path.split
    pjoin  = os.path.join
    
    CConfigDir = 'conf'
    
    if len(args) != 1:
        print('Usage: script <repository path>')
        exit()
        
    def Exec(cmd, stageName):
        res = OsExec(cmd, True)
        if res != 0: raise UserException("Stage '{0}' error!".format(stageName))
        
    reposPath = args[0]
    reposShortName = psplit(reposPath)[1]
    reposUrl = 'svn://127.0.0.1/' + reposShortName 
    tmpDir = tempfile.mkdtemp()

    # 1. copy config to temp
    tmpConfig = pjoin( tmpDir, CConfigDir )
    reposConfig = pjoin(reposPath, CConfigDir)
    shutil.copytree(reposConfig, tmpConfig)
    
    # 1a. remove read-only
    def RemoveReadOnly(path):
        fullPath = pjoin(reposPath, path)
        os.chmod(fullPath, stat.S_IREAD | stat.S_IWRITE)
    
    RemoveReadOnly('format')
    RemoveReadOnly('db/format')
        
    # 2. svn export
    tmpExportPath = pjoin(tmpDir, reposShortName)
    cmd = 'svn export {url} "{path}"'.format(url = reposUrl, path = tmpExportPath)
    Exec(cmd, 'export')
    
    #3. rm repository
    shutil.rmtree(reposPath)
    
    #4. create repository
    Exec( 'svnadmin create "{0}"'.format(reposPath), 'create' )
    
    #5. restore config
    shutil.rmtree(reposConfig)
    shutil.copytree(tmpConfig, reposConfig)
    
    #6. import
    cmd = 'svn import "{path}" {url} -m "{msg}"'.format(path = tmpExportPath, 
                                                        url = reposUrl, 
                                                        msg = 'clean up')
    Exec(cmd, 'import') 
    
    #7. clean temp
    shutil.rmtree(tmpDir) 
    
    print('Done!')

# --------------------------------------------

if __name__ == '__main__':
    Main(sys.argv[1:])
    
    