'''
Created on May 11, 2010

@author: root
'''

from QConfigMain.GuiDeviceConfigType import ConfigMaker, MonitorMaker
        
# -----------------------------------------------------------

def MakeConfig(m : ConfigMaker) -> None:
    Field = ConfigMaker.Field
    
    f = Field('BoardName', None, m.BoardName)
    f.Validator = m.vn()
    f.UseInStopScript = True
    f.ToolTip = ''
    f.Help = ''
    
    m.Add(f)
    
    f = Field('HwNumber', None, m.HwNumber)
    f.Validator = m.vi()
    f.ToolTip = ''
    f.Help = ''
    m.Add(f)

#    using BoardName instead
# 
#    f = Field('GwName', None, m.BoardName)
#    f.Validator = m.vn()
#    f.UseInStopScript = True
#    f.Help = ''
#    m.Add(f)
    
    f = Field('GwPilotNumber', 'GwPilotNumber', None)
    f.Validator = m.vi()
    f.ToolTip = 'Allowed symbols: 0-9'
    f.Help = ''
    m.Add(f)
    
    f = Field('GwCalledAddrType', 'GwCalledAddrType', 'RequestUri')
    f.Enum = ['RequestUri', 'To']
    f.Validator = m.vn()
    f.RuntimeCommand = '.CalledAddrType'
    f.ToolTip = ''
    f.Help = ''
    m.Add(f)
    
    f = Field('GwCallingAddrNoDigit', 'GwCallingAddrNoDigit', 'ReleaseCall')
    f.Enum = ['ReleaseCall', 'UsePilotNumber']
    f.Validator = m.vn()
    f.RuntimeCommand = '.CallingAddrNoDigit'
    f.ToolTip = ''
    f.Help = ''
    m.Add(f)
    
    f = Field('GwCallingAddrType', 'GwCallingAddrType', 'From')
    f.Enum = ['Contact', 'From']
    f.Validator = m.vn()
    f.RuntimeCommand = '.CallingAddrType'
    f.ToolTip = ''
    f.Help = ''
    m.Add(f)
        
    f = Field('SipCallTraceLevel', 'SipCallTraceLevel', 3)
    f.RuntimeCommand = '.Sip.LogVevel'
    f.ToolTip = ''
    f.Help = ''
    m.Add(f)
    
    f = Field('SipExternalIP', 'SipExternalIP', '83.85.98.198')
    f.Validator = m.vh(ip = True, ip_port = False)
    f.ToolTip = 'Format: 0.0.0.0 (IP)'
    f.Help = ''
    m.Add(f)
    
    f = Field('SipLocalHost', 'SipLocalHost', '')
    f.Validator = m.vh(ip = True, port = True)
    f.ToolTip = 'Format: 0.0.0.0:0 (IP:port) or 0.0.0.0 (IP) or 0 (port)'
    f.Help = ''
    m.Add(f)
    
    f = Field('SipProxyHost', 'SipProxyHost', '192.168.0.240:5060')
    f.Validator = m.vh(ip = True)
    f.ToolTip = 'Formats: 0.0.0.0:0 (IP:port) or 0.0.0.0 (IP)'
    f.Help = ''
    m.Add(f)
    
    f = Field('SipProxyUserName', 'SipProxyUserName', '')
    f.Validator = m.vn(True)
    f.ToolTip = 'Allowed symbols: A-Z, a-z, 0-9, _.\nMust start with letter'
    f.Help = ''
    m.Add(f)
    
    f = Field('SipProxyPassword', 'SipProxyPassword', '')
    f.ToolTip = ''
    f.Help = ''
    m.Add(f)
    
    f = Field('SipProxyRegistrationInd', 'SipProxyRegistrationInd', True)
    f.Validator = m.vb()
    f.ToolTip = ''
    f.Help = ''
    m.Add(f)
    
    f = Field('IsdnHwType', 'IsdnHwType', 'E1')
    f.Enum = ['E1', 'T1']
    f.Validator = m.vn()
    f.ToolTip = ''
    f.Help = ''
    m.Add(f)
    
    f = Field('IsdnSide', 'IsdnSide', 'TE')
    f.Enum = ['NT', 'TE']
    f.Validator = m.vn()
    f.ToolTip = ''
    f.Help = ''
    m.Add(f)
    
#    f = Field('IsdnInBlockingCh', 'IsdnInBlockingCh', '')
#    f.Help = ''
#    m.Add(f)
#    
#    f = Field('IsdnOutBlockingCh', 'IsdnOutBlockingCh', '')
#    f.Help = ''
#    m.Add(f)
    
    f = Field('IsdnL2Trace', 'IsdnL2Trace', False)
    f.Validator = m.vb()
    f.RuntimeCommand = '.Trunk1.L2TraceInd'
    f.ToolTip = ''
    f.Help = ''
    m.Add(f)
    
    f = Field('IsdnRestartReq', 'IsdnRestartReq', True)
    f.Validator = m.vb()
    f.ToolTip = ''
    f.Help = ''
    m.Add(f)
    
    f = Field('IsdnSendAlertingTone', 'IsdnSendAlertingTone', False)
    f.Validator = m.vb()
    f.ToolTip = ''
    f.Help = ''
    m.Add(f)
    
    f = Field('TraceLevel', 'TraceLevel', 3)    
    f.RuntimeCommand = '.LogLevel'
    f.ToolTip = ''
    f.Help = ''
    m.Add(f)
    
    f = Field('StartRtpPort', 'StartRtpPort', 8000)
    f.Validator = m.vp()
    f.ToolTip = ''
    f.Help = ''
    m.Add(f)
    
    f = Field('UseALow', 'UseALow', True)
    f.Validator = m.vb()
    f.ToolTip = ''
    f.Help = ''
    m.Add(f)
    
    f = Field('ResetOnFatalError', 'ResetOnFatalError', False)
    f.Validator = m.vb()
    f.ToolTip = ''
    f.Help = ''
    m.Add(f)
    
    f = Field('WatchdogEnable', 'WatchdogEnable', False)
    f.Validator = m.vb()
    f.ToolTip = ''
    f.Help = ''
    m.Add(f)
    
    f = Field('LiuSyncMaster', None, False)
    f.Validator = m.vb()
    f.ToolTip = ''
    f.Help = ''
    m.Add(f)
    
    m.Close(version = 1, typeID = 1, typeName = 'E1')

# -------------------------------------------------------------------    

def MakeMonitor(m : MonitorMaker):
    
    m.Add('resGwCalls',              '.Calls')
    m.Add('resGwInfo',               '.Info')
    m.Add('resIsdnState',            '.Trunk1.State')
    m.Add('resIsdnActivationCount',  '.Trunk1.ActivationCount')
    m.Add('resIsdnLastActivation',   '.Trunk1.LastActivation')
    m.Add('resIsdnLastDeactivation', '.Trunk1.LastDeactivation')
        
# -----------------------------------------------------------

    
if __name__ == "__main__":
    pass
