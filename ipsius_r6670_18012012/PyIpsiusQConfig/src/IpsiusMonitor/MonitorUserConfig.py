'''
Created on 24.11.2010

@author: Skiv
'''

def SetupUserCfg(setHost, addWidget):
    
    setHost('192.168.0.240', 50200, 'Igor')
    addWidget('Finder', refreshSec = 130)
    addWidget('Custom', objName = "HwFinder", cmds = "ListAliases()", caption = 'Aliases')
       
    addWidget('DevState', objName = "ipsius.Trunk1.i0", caption = "Tech state info.", refreshSec = 15)     
    addWidget('DevCalls', objName = "ipsius.Trunk2.i0", caption = 'Calls.')
    addWidget('Dev', objName = "ipsius.Trunk2.i0", caption = 'State + Calls sum.', refreshSec = 30)      
    
    setHost('127.0.0.1', 50200, 'Local')
    addWidget('Finder')
    
       
         
