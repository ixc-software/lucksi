'''
Created on 08.11.2010
@author: Valkiriy
'''

def SetupUserCfg(h, w):
    
    h('192.168.0.110', 50200, 'Igor')    
    w("Finder")
    w("Dev", "ipsius.Trunk1.i0")
    
    h('localhost', 3220)
    w('Custom', '', '', refreshSec = 90)
    w('Finder')
    
    h('abb', 3221)
    w('Custom', '', '')

    
    