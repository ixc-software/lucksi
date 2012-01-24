'''
Created on Mar 29, 2010

@author: root
'''

from QConfigMain.GuiDeviceWidget import WidgetMaker

# ---------------------------------------------------------------

def CustomizeWidget(m : WidgetMaker) -> None:
    
    def PrintX() -> None: 
        m.ChangeStatus("Printing 'X' ... ")
        print('X')
    
    m.AddToolBar('toolBar', 29)
    m.AddToolBarBtn('toolBar', 'PrintX', PrintX, True, True, True)
     
    m.SetupShowWidgetBtn('btnShowInfo', 'showInfo', [], False, 'Show', 'Hide')
    
    m.EnableWidget('host', False, False)
