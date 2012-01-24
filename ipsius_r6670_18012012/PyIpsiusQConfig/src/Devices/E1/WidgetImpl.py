'''
Created on Feb 12, 2010

@author: root
'''

from QConfigMain.GuiDeviceWidget import WidgetMaker

# ---------------------------------------------------------------

def CustomizeWidget(m : WidgetMaker) -> None:
    
    resizeList = ['groupGwMonit']
    hideOnStartUp = True
    m.SetupShowWidgetBtn('btnGwInfo', 'resGwInfo', resizeList, hideOnStartUp)
    m.SetupShowWidgetBtn('btnGwCalls', 'resGwCalls', resizeList, hideOnStartUp)
    
    