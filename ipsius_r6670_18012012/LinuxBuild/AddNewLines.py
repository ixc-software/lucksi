
import os, sys

from CDUtilsPack.MiscUtils import GetScriptDir

# -----------------------------------------------------------

def AddNewLines(dir : str, excludeSubDirs : [str], silentMode : bool) -> None:
    
    def pr(text : str):
        if not silentMode: print(text)
    
    for dir, subDirs, files in os.walk(dir):
        skip = False
        for exclDir in excludeSubDirs:
            if dir.find(exclDir) >= 0: 
                skip = True
                break
        
        if skip: continue
        
        pr ("Checking directory: " + dir)
        for fileName in files:       
            if fileName.endswith(".h") or fileName.endswith(".cpp"):
                file = os.path.join(dir, fileName)
                text = ""
                with open(file, 'r') as f:
                    text = f.read()
    
                if text.endswith('\n'): continue
                
                retcode = os.system("chmod +w " + file)
                if retcode != 0:
                    pr ("chmod returned {0} on {1}".format(retcode, file))
                    continue
                
                with open(file, 'a') as f:
                    f.write("\n")
                pr("Add new line to file: " + fileName)

# -----------------------------------------------------------

if __name__ == "__main__":
    
    exclude = ['.svn']

    silentMode = False

    if len(sys.argv) == 2 and sys.argv[1] == "/silent":
        silentMode = True
    elif len(sys.argv) > 1:
        print ("Warning: Unknown parameters will be ignored!")
    
    AddNewLines(GetScriptDir("../Src"), exclude, silentMode)    
    AddNewLines(GetScriptDir("../Blackfin"), exclude, silentMode)
    
    
    