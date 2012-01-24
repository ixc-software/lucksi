'''
Created on May 20, 2010

@author: root
'''
from QConfigMain.GuiDeviceConfigType import ConfigMaker, MonitorMaker


def MakeConfig(m : ConfigMaker) -> None:
    
    Field = ConfigMaker.Field
    
    f = Field('BoardName', None, m.BoardName)
    f.UseInStopScript = True	
    f.Validator = m.vn()
    f.Help = 'help'
    m.Add(f)
    
    f = Field('Trace', 'runTrace', True)
    f.Validator = m.vb()
    m.Add(f)
    
    m.Close(version = 1, typeID = 2, typeName = 'AOZ')

# -------------------------------------------------

def MakeMonitor(m : MonitorMaker) -> None:
    
    m.Add('monitData1', 'CS_Set')    
    