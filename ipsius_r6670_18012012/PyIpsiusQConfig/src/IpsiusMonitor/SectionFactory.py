'''
Created on 13.11.2010

@author: Valkiriy
'''

from PyQt4 import QtCore, QtGui
from DRIDomain.TelnetClientDriAsync import CommandDriHandler
from Controls import *
from types import FunctionType
from CDUtilsPack.Telnet.TelnetParams import TelnetParams

from CDUtilsPack.MiscUtils import LoadModuleFromFile
from CDUtilsPack.MetaUtils import UserException
    
class ISection():    
    
    '''
        _Section interface
    '''
    
    def BindStatusBar(self, statusBar): self.__Assert() # statusBar : MainWindow._StatusBar       
    def Enable(self, driReqFn): self.__Assert() # driReqFn: Fn(cmdList : str or [str], callback)
    def OnExpand(self, exp : bool): self.__Assert()           
    def OnRefresh(self): self.__Assert()
    def GetText(self): self.__Assert()          
    
    def __Assert(self): raise AssertionError('Must be override')


'''Section detail'''
  
def _SimplestParser(response : CommandDriHandler) -> str: #errIndFn(bool ok)              
    if not response.ResultsOK:
        return "Response error!\nErrType: {0}\nErr: {1}"\
        .format(response.LastResult.ErrorType, response.LastResult.Error)                           
       
    res = ''
    
    for i in response.Results:        
        res += "{}\n".format(i.Text)
    return res[:-1]       

#------------------------------------------------------------

def _FixCaption(wdName : str, caption : str, objName : str):
    if not caption: caption = wdName + ' ' + objName
    if not caption: raise Exception('Wrong caption or objName')
    return caption

#------------------------------------------------------------

class _LabelReflection(QtGui.QLabel):
    
    '''
        Simple reflection widget with customized parser
    '''    
    
    def __init__(self, parserFn = _SimplestParser): # todo add parser here
        super().__init__()
        font = QtGui.QFont()
        font.setFamily('Courier New')
        self.setFont(font)
        self.setTextInteractionFlags(QtCore.Qt.TextSelectableByMouse)
        self.setWordWrap(True)
        
        self.parserFn = parserFn
        self.IndInProgress(False)        
        
    def Reflect(self, res : CommandDriHandler):
        ok = True
        text = ''
        try: 
            text = self.parserFn(res)
        except Exception as e:
            ok = False
            text = str(e)
        self.setText(text)
        
    def getText(self) -> str :
        return self.text()
    
    def IndInProgress(self, inProg : bool):        
        light = '#E8E8E0' #'#DAD6CD'#'#E8E8DD'
        dark = '#AAAAA3'
        color = dark if inProg else light         
        self.setStyleSheet("QWidget { background-color: %s }" % color)
        
#------------------------------------------------------------  
           
class _Section(QtGui.QGroupBox, ISection):        
    
    '''   
        Common Gui of reflection widgets                     
    '''    
    
    def __init__(self, common : {}, # common parameters created in function CreateSectionsByCfg 
                 reflection : QtGui.QWidget, caption : str,
                 refreshSec : int,                 
                 cmd : str or [str]):
        
        super().__init__()                                    
        
        if caption == None:
            raise UserException("Wrong parameter ('caption' = {})".format(caption))
        if refreshSec != None and refreshSec <= 0: 
            raise UserException("Wrong parameter ('refreshSec' = {})".format(refreshSec))
        
        self.__caption = '[{0}] [{1}]'.format( common['hostCaption'], caption )                       
        
                    
        # set and adjust layout
        self.setLayout(QtGui.QVBoxLayout())                              
        self.layout().setSpacing(0)        
        self.layout().setContentsMargins(0, 0, 0, 0)                                  
        
        self.__panel = self.__CreatePanel(common['imgs'])
        self.layout().addWidget(self.__panel)
        
        line = QtGui.QFrame()
        line.setFrameStyle(QtGui.QFrame.Sunken)
        line.setFrameShape(QtGui.QFrame.HLine)        
        self.layout().addWidget( line )                                  
                        
        self.__reflection = reflection
        self.layout().addWidget(self.__reflection)   
               
        self.__timer = Timer(refreshSec * 1000, self.OnRefresh) if refreshSec else None 
        
        self.__cmd = cmd        
        self.__driReqFn = None      
    
    def __CreatePanel(self, imgs : {str:QtGui.QPixmap}):        
        p = Panel(self.__caption)
        
        p.AddWidget(QtGui.QLabel('  '), False)                                
                
        p.connInd = QtGui.QLabel(' NotConnected! ')
        p.connInd.setFrameStyle(QtGui.QFrame.Box)                              
        #p.connInd.setStyleSheet("QWidget { background-color: %s }" % '#ff0000') # '#ff0000' - red
        p.connInd.setVisible(True)
        p.AddWidget(p.connInd, False)
        
        p.AddWidget(QtGui.QLabel('  '), False)
        
        p.errInd = QtGui.QLabel(' Error! ')
        p.errInd.setStyleSheet("QWidget { background-color: %s }" % '#ff0000') # '#ff0000' - red                      
        p.errInd.setVisible(False)
        p.errInd.setFrameStyle(QtGui.QFrame.Box)
        p.AddWidget(p.errInd, False)                      
        
        p.layout().addStretch(1)    
               
        p.elapsedTime = ElapsedTime()                                  
        p.AddWidget(p.elapsedTime, False)
               
        bCopy = PanelButton(imgs['Clipboard'])
        p.AddWidget(bCopy, True, True, lambda: QtGui.QApplication.clipboard().setText(self.GetText()))        
        
        bRefresh = PanelButton(imgs['Refresh'])
        p.AddWidget(bRefresh, True, True, self.OnRefresh)
        
        p.isExpand = True
        bExp = PanelButton(imgs['Minus'])#QtGui.QPushButton('-')
        def OnExpComplete(exp):
            # change view            
            if p.isExpand == exp: return          
            img = imgs['Minus'] if exp else imgs['Plus']
            bExp.SetIcon(img)
            p.isExpand = exp            
            
        p.OnExpComplete = OnExpComplete                    
        p.AddWidget(bExp, True, True, lambda: self.OnExpand(not p.isExpand))                

        return p         
    
    def BindStatusBar(self, statusBar): #statusBar : MainWindow._StatusBar
        assert(not hasattr(self, '__status'))
        assert(statusBar)
        self.__status = statusBar
        self.__status.IncWdAll(True)
        
    def __StErr(self):
        return self.__panel.errInd.isVisible()
            
    def __SetStateErr(self, err : bool): 
        assert(self.__driReqFn != None) #connected
        oldErrState = self.__StErr()        
        self.__panel.errInd.setVisible(err)   
        
        if oldErrState != err:            
            self.__status.IncWdOk(not err)
            self.__status.IncWdErr(err) 
            
        if (err):           
            self.__status.IncTotalDriErr(True)                                      
        else:         
            self.__status.IncTotalDriOk(True)                  
                                
        
    def Enable(self, driReqFn):               
        
        enabled = driReqFn != None
        oldConnectState = (self.__driReqFn != None)                      
        
        self.__panel.connInd.setVisible(not enabled)        
        if self.__timer: self.__timer.Enable(enabled)        
        self.__driReqFn = driReqFn                                       
        
        if enabled != oldConnectState: # update status if connection state changed            
            self.__status.IncWdConnected(enabled)
            if not self.__StErr(): self.__status.IncWdOk(enabled)
            
        self.OnRefresh()
        
    def OnExpand(self, exp : bool):
        self.__reflection.setVisible(exp)
        self.__panel.OnExpComplete(exp)
    
    def OnRefresh(self):                        
        
        def OnRsp(res : CommandDriHandler):
            reflectComplete = True  
            try: self.__reflection.Reflect(res)
            except Exception as e:
                print(e)
                reflectComplete = False
                        
            ok = res.ResultsOK and reflectComplete
            self.__SetStateErr(not ok)
                            
            self.__panel.elapsedTime.Reset()
            self.__reflection.IndInProgress(False)     
            
        if self.__driReqFn == None: return 
        
        self.__reflection.IndInProgress(True)                                                                                   
        self.__driReqFn(self.__cmd, OnRsp)                     
              
    def GetText(self):
        connectStatus = 'not connected' if self.__driReqFn == None else 'connected'
        errStatus = 'Error!' if self.__panel.errInd.isVisible() else 'NoError'                
        return "Widget '{0}' [Status: {1}, {2}]:\n{3}".format(self.__caption, connectStatus, errStatus, self.__reflection.getText())                
    

#-----------------------------------------------------------------------------------      

class _WidgetTypes:
    
    ''' Dictionary of section customizer function described in user config '''         
        
    @staticmethod
    def CreateFinder(common : {}, caption = 'Finder', refreshSec = None) -> _Section:
                        
        reflection = _LabelReflection(_SimplestParser)        
                
        newWidget = _Section(common, reflection, caption, refreshSec, "HwFinder.ListBoards true")
        return newWidget                   
    
    @staticmethod
    def CreateDevState(common : {}, objName, caption = None, refreshSec = None):
        
        # request about NObjDss1Interface or NObjAozBoard        
        
        reflection = _LabelReflection()      
        caption = _FixCaption('DevState', caption, objName)
        
        newWidget = _Section(common, reflection, caption, refreshSec, objName + ".StateInfo")
        return newWidget
    
    @staticmethod
    def CreateDevCalls(common : {}, objName, caption = None, refreshSec = None):        
        # request .StateInfo about NObjDss1Interface or NObjAozBoard        
        
        reflection = _LabelReflection()           
        caption = _FixCaption('DevCalls', caption, objName)
        
        newWidget = _Section(common, reflection, caption, refreshSec, objName + ".Calls")
        return newWidget
    
    @staticmethod
    def CreateDev(common : {}, objName, caption = None, refreshSec = None):
        # request .State, .Calls about NObjDss1Interface or NObjAozBoard                                                           
                
        
#        def Parser(response : CommandDriHandler) -> str :
#            if not response.ResultsOK:
#                return "Response error!\nErrType: {0}\nErr: {1}"\
#                .format(response.LastResult.ErrorType, response.LastResult.Error)
#                
#            if not len(response.Results) == 2: raise Exception('Unexpected response count' )                                                
#           
#            res = ''             
#            res += "State: {}\n".format(response.Results[0].Text)
#            calls = response.Results[1].Text
#            begin = calls.rfind('\n') + 1
#            res += "Calls: " + calls[begin:]   
#            return res
                                  
            
        
        reflection = _LabelReflection()        
        caption = _FixCaption('Dev', caption, objName)
                
        cmds = [objName + ".State", objName + ".Calls(true)"]
        #cmds = [objName + ".State", objName + ".Calls"]
        newWidget = _Section(common, reflection, caption, refreshSec, cmds)
        return newWidget
    
    @staticmethod
    def CreateCustom(common : {}, cmds, objName = None, caption = None, refreshSec = None):
        
        caption = _FixCaption('Custom', caption, objName)                      
        
        reflection = _LabelReflection()          
                
        cmdList = [cmds] if isinstance(cmds, str) else cmds
        
        fullCmd = []
        if objName == None: 
            fullCmd = cmds
        else:               
            for cmd in cmdList: fullCmd.append(objName + '.' + cmd)                
        
        newWidget = _Section(common, reflection, caption, refreshSec, fullCmd)
        return newWidget   
    

from CDUtilsPack.MetaUtils import Property
class Item:

    '''
    Dri info + list of _Section
    '''

    def __init__(self, addr : TelnetParams, addrCaption : str):
        self.addr = addr
        self.addrCaption = addrCaption
        self.sections = []
    
    def AddSection(self, section : _Section):
        assert( isinstance(section, _Section) )
        self.sections.append(section)
    
    Addr = Property('addr')
    AddrCaption = Property('addrCaption')
    Sections = Property('sections')                    

#---------------------------------------------------------------------------

def CreateSectionsByCfg(absFileName : str, imgs : {str: QtGui.QPixmap}) -> [Item]:  

    result = [] #[Item]
    
    common = {}
    common['imgs'] = imgs     
    
    lineNum = 0 
    
    def Create(t, *wdPosArgs, **wdKeyArgs) -> _Section: # find concrete creator and create section
        fnName = 'Create' + t        
        createFn = getattr(_WidgetTypes, fnName, None)
        if not createFn: raise UserException('Widget type ' + t + ' not exist.')        
        return createFn(common, *wdPosArgs, **wdKeyArgs)
    
    def VerifyType(obj, type):
        if not isinstance(obj, type):
            raise Exception("Value '{0}' expected as {1}.".format(obj, type))                    
    
    def SetHost(host : str, port : int, caption = None, login = 'Void', pwd = 'Void'):        
        port = int(port)
        if caption: VerifyType(caption, str)            
        VerifyType(login, str)
        VerifyType(pwd, str)
        if caption == None: caption = '{}:{}'.format(host, port)               
        common['hostCaption'] = caption        
        result.append( Item( TelnetParams(host, port, login, pwd),  caption) )
        nonlocal lineNum
        lineNum += 1                      
       
    def AddWd(t, *args, **keyArgs):
        if len(result) == 0: raise UserException('Set host before add widget.')        
        wd = Create(t, *args, **keyArgs)
        assert(wd)        
        result[len(result) - 1].AddSection(wd)
        nonlocal lineNum
        lineNum += 1                                 
    
    assert(absFileName) 
        
    try:
        moduleCfg = LoadModuleFromFile( absFileName )
        moduleCfg.SetupUserCfg(SetHost, AddWd)        
    except AssertionError as e:
        raise e    
    except ImportError as e:        
        raise  UserException('Can`t read config. Reason: {}'.format(str(e)))    
    except Exception as e:        
        msg = "Read config error at line #{0} of SetupUserCfg. Reason: {1}"\
        .format( lineNum, str(e) )        
        raise  UserException(msg)
    
    return result


# ---------------------------------------------------


if __name__ == "__main__":    
    pass
    
    