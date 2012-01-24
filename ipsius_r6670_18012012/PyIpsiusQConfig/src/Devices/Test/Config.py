'''
Created on May 20, 2010

@author: root
'''

from QConfigMain.GuiDeviceConfigType import ConfigMaker, MonitorMaker

# -----------------------------------------------

def MakeConfig(m : ConfigMaker) -> None:
    Field = ConfigMaker.Field
#    

#        self.UseInStopScript = False

#        
#        self.Enum = None # [str]
#        self.Help = ''
    
    # field with no widget, used in scripts
    f = Field('StaticName', None, m.BoardName)
    f.NeedSaveToFile = True
    f.UseInRunScript = True
    f.UseInStopScript = True
    f.RuntimeCommand = ''  
    f.Validator = m.vn()
    f.ToolTip = ''
    f.Help = 'Board non-changeable name.'
    m.Add(f)
    
    # field with widget, doesn't used in scripts
    f = Field('Name', 'name', m.BoardName)
    f.NeedSaveToFile = True
    f.UseInRunScript = False
    f.RuntimeCommand = ''
    f.UseInStopScript = False
    f.Validator = m.vn()
    f.ToolTip = 'Use only A-Z, a-z, 0-9 and _; start with letter.'
    f.Help = 'Board changeable name (given by user alias).'
    m.Add(f)
        
    # using QComboBox as bool
    f = Field('Trace1', 'trace1', True) # QComboBox
    m.Add(f)
    
    # using QComboBox as enum
    f = Field('State1', 'state1', 'State13') # QComboBox
    f.Enum = ['State11', 'State12', 'State13']
    m.Add(f)
        
    # using QGroupBox with QRadioButtons as bool
    f = Field('Trace2', 'trace2', False)
    f.Validator = m.vb()
    m.Add(f)
    
    # using QGroupBox with QRadioButtons as enum
    f = Field('State2', 'state2', 'State21') # QGroupBox with QRadioButtons
    f.Enum = ['State21', 'State22']
    m.Add(f)
    
    # using QCheckBox as bool
    f = Field('Trace3', 'trace3', True)
    f.Validator = m.vb() 
    m.Add(f)
    
    # using QCheckBox as enum
    f = Field('State3', 'state3', 'Active') # QCheckBox
    f.Enum = ['Active', 'NotActive']
    m.Add(f)
    
    f = Field('Host', 'host', '127.0.0.1:1258')
    f.Validator = m.vh()
    m.Add(f)
    
    f = Field('Ip', 'ip', '168.152.10.0')
    f.Validator = m.vh(ip = True, ip_port = False)
    f.ToolTip = "Format: 0.0.0.0"
    m.Add(f)
    
    f = Field('Port', 'port', 45681)
    f.Validator = m.vp()
    m.Add(f)

    m.Close(version = 1, typeName = 'Test', typeID = 1001)

# -----------------------------------------------

def MakeMonitor(m : MonitorMaker) -> None:
    
    m.Add('btnShowInfo', 'CS_Print "showInfo"')
    