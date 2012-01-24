'''
Created on 16.11.2010
@author: Valkiriy

IpsiusMonitor startup code.
Set libPath, parse cmdLine, load icon and etc., read user config and create widgetList

'''

import os
from PyQt4 import QtGui

''' StartupDetail '''

def _CmdLineParser(cmdLine : [str]) -> {}:
    
    cmdLine = cmdLine[1:]        
    from CDUtils import CmdOptions
    
    opts = CmdOptions(autoAddShortNames = True)   

#    opts.Add('ip', "Ipsius telnet host address", ('telnetHost:str',), req = True)   #/ip
#    opts.Add('port', "Ipsius telnet port number", ('port:int',), req = True)        #/p                  
#    opts.Add('pwd', "Ipsius telnet password", ('pwd:str*',), req = False)           #/pwd
#    opts.Add('login', "Ipsius telnet login", ('login:str*',), req = False)          #/l
    opts.Add('cfg', "User config file", ('cfg:str',), req = False)                  #/c
    
    result = opts.Parse( cmdLine, exitOnHelp = True)       
    
    #fix default
    #if 'pwd' not in result: result['pwd'] = 'VoidPwd'
    #if 'login' not in result: result['login'] = 'VoidLogin'    
    
    return result


def _ImageLoader(dir) -> {str : QtGui.QPixmap} : # can raise if not exist
    
    from CDUtilsPack.MetaUtils import UserException
    
    dir = GetScriptDir('../../' + dir)
    if not os.path.exists(dir): raise UserException('Dir not exist: ' + dir)
    result = {}
       
    def Add(keyInResult : str, fileName : str):
        pixmap = QtGui.QPixmap()       
        abs = os.path.join(dir, fileName)
        if not pixmap.load( abs ):
            raise UserException('File not exist: ' + abs)                  
        result[keyInResult] = pixmap                                  
        
    Add('Refresh', 'Refresh.png')
    Add('Plus', 'Plus.png')
    Add('Minus', 'Minus.png')
    Add('Clipboard', 'Clipboard.png')
    Add('App', 'app.png')

    return result    

#---------------------------------------------------------------------------------

from PyQt4 import QtCore
from types import FunctionType
class Foo(QtCore.QObject):
    
    def __init__(self):
        self.connect_and_emit_trigger()

    # Define a new signal called 'trigger' that has no arguments.
    trigger = QtCore.pyqtSignal(FunctionType)

    def connect_and_emit_trigger(self):
        def handle_trigger():
            # Show that the slot has been called.
            print ("trigger signal received")
        
        # Connect the trigger signal to a slot.
        self.trigger.connect(handle_trigger)

        # Emit the signal.
        self.trigger.emit()

    

def _Run(argv):
    
    from CDUtilsPack.MetaUtils import UserException
    from PackUtils.CoreBaseException import CoreBaseException     
    from SectionFactory import CreateSectionsByCfg
    from MainWindow import MainWindow
        
    try:
        appLoop = QtGui.QApplication(argv)  # for Qt widgets using 
        
        imgs = _ImageLoader(dir = 'Images')
        sett = _CmdLineParser(argv)        
                
        absFileName = os.path.join(os.getcwd(), sett['cfg'])
        del sett['cfg']                       
        sectionDict = CreateSectionsByCfg(absFileName, imgs)        
        
        app = MainWindow(sectionDict, imgs)        
        app.show()                                      

        appLoop.exec()             
                   
    except UserException as e:
        print ("Aborted!\nStart-up error:", e)        
    
    except CoreBaseException as e:
        print("Aborted!\n{0}: {1}".format(type(e).__name__, e))
        
#    except Exception as e:
#        print("Exception occured!\n{0}: {1}".format(type(e).__name__, e))                
    

if __name__ == "__main__":
    
    #Add ./src path to PYTHONPATH.      
    import sys             
    from CDUtilsPack.MiscUtils import DirUp, GetScriptDir
    sys.path.append( DirUp(GetScriptDir(), 1) )    
    
    # run
    _Run(sys.argv)
