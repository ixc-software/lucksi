'''
Created on 31.10.2010

@author: Valkiriy
'''

import os
from PyQt4 import QtGui, QtCore
from Controls import *
from PackUtils.CoreTrace import CoreTrace
from CDUtilsPack.Telnet.TelnetSocket import SocketError
from SectionFactory import Item

from DRIDomain.TelnetClientDriAsync import TelnetClientDriAsync, CommandDriHandler
from CDUtilsPack.Telnet.TelnetParams import TelnetParams
from DRIDomain.TelnetClientDriAsyncQueue import TelnetClientDriAsyncQueue

class _DockContent(QtGui.QWidget):
    
    ''' 
        Gui component of main window (control panel + text output).        
    '''
    
    __TimeFormat = 'hh:mm:ss'
    __Red = '#ff0000'
    __Green = '#00ff00'
    
    def __init__(self,           
                 imgs : {str : QtGui.QPixmap},                      
                 expAllFn, # Fn(exp : bool)
                 refrshAllFn, # Fn()
                 copyAllFn # Fn() 
                 ):
        super().__init__()
        self.setLayout(QtGui.QVBoxLayout())
        self.layout().setSpacing(0)        
        self.layout().setContentsMargins(2, 0, 4, 0)                                      
            
        self.__panel = self.__CreatePanel(expAllFn, imgs, refrshAllFn, copyAllFn)                                       
        self.layout().addWidget(self.__panel)      
        
        # add text browser
        textOut = QtGui.QTextBrowser()
        policy = textOut.sizePolicy()
        policy.setVerticalStretch(50)
        textOut.setSizePolicy(policy)
        self.layout().addWidget(textOut)
        textOut.setMinimumHeight(20)
                
        # top align
        self.layout().addStretch(1)          
        
        #Create interface:
        def TextOut(text):
            time = QtCore.QDateTime.currentDateTime().toString(self.__TimeFormat)  
            textOut.append(time + ' ' + text) 
        
        self.textOutFn = TextOut
        #self.indConnectFn = panel.IndSwitch         
    
    def AddTelnetBt(self, caption : str, telnet : TelnetParams) -> FunctionType:         
        bTelnet = PanelButton(name = caption,
                 toolTip = 'Open telnet console at {0}:{1}'.format(telnet.Host, telnet.Port))
        runTelnetCmd = "start telnet /a {0} {1}".format(telnet.Host, telnet.Port)
        
        def ConnectInd(on):
            #ind.Switch(on)
            bTelnet.setEnabled(on)                           
            color = _DockContent.__Green if on else _DockContent.__Red            
            bTelnet.setStyleSheet("QWidget { background-color: %s }" % color)                         
            
        self.__panel.AddWidget(bTelnet, False, False, lambda: os.system(runTelnetCmd))
        ConnectInd(False)
        return ConnectInd
        
    @staticmethod
    def __CreatePanel(expFn, imgs : {str:QtGui.QPixmap}, refreshAll, copyAllFn): #expFn(bool)
        p = Panel('DriConnection')   
                
        p.layout().addStretch(10)                                                

        bRefreshAll = PanelButton(imgs['Refresh'], 'RefreshAll', toolTip = 'Refresh all')     
        
        bCopy = PanelButton(imgs['Clipboard'], toolTip = 'Copy all widget info to clipboard')
        p.AddWidget(bCopy, True, True, copyAllFn)              
        
        p.AddWidget(bRefreshAll, False, True, refreshAll)                       

        bPlus = PanelButton(imgs['Plus'], toolTip = 'Expand all')
        bMinus = PanelButton(imgs['Minus'], toolTip = 'Colapse all')
        
        p.AddWidget(bPlus, True, True, fn = lambda: expFn(True) )
        p.AddWidget(bMinus, True, True, fn = lambda: expFn(False) )                    
        
        return p   
    
class _StatusBar(QtGui.QStatusBar):    
    
    def __init__(self):
        super().__init__()                                   
        
        def CreateCounter(caption):
            wd = QtGui.QLabel()
            counter = -1
            def Fn(inc : bool):
                nonlocal counter
                counter += 1 if inc else -1
                wd.setText('{0}: {1}'.format(caption, counter))
            self.addPermanentWidget(wd)
            Fn(True) # set 0
            return Fn
        
        self.IncTotalDriOk = CreateCounter('TotalDriCmdOk') 
        self.IncTotalDriErr = CreateCounter('TotalDriCmdError') 
        
        self.IncWdAll = CreateCounter('WdAll') 
        self.IncWdOk = CreateCounter('WdOk') 
        self.IncWdConnected = CreateCounter('WdConnected')      
        self.IncWdErr = CreateCounter('WdErr')   
                                    
        

class _Dri:
    '''
        Dri + AsyncQueue + connect indicator
    '''
    def __init__(self, telnet : TelnetParams, callbackFn, indFn):
        
        def OnDriError(e : Exception): pass 
        def LogOuter(self, obj, text : str, textID : str = None):pass   
        
        def Connect(telnet : TelnetParams, ok):   
            process = self.__driQueue.Process if ok else None            
            callbackFn(telnet, process)
            indFn(ok)        
         
        self.__dri = TelnetClientDriAsync(None, telnet, OnDriError, CoreTrace(LogOuter, True), Connect, True)
        self.__driQueue = TelnetClientDriAsyncQueue(self.__dri, None)        
   

class MainWindow(QtGui.QMainWindow):
    '''
        IpsiusMonitor main window. Gui + dri connection
    '''    
        
    __PollMsec = 5000    

    def __init__(self,
                 sections : [Item],
                 icons : {str : QtGui.QPixmap},
                 ):        
                    
        super().__init__()                                                                                                                           
        
        for item in sections: assert( isinstance(item, Item))                                               
        
        #create gui and bind controls at OnXxx - processors
        self.__SetupGui(icons['App'], sections, icons)                         
        
        self.__sects = sections        
        self.__dri = []
        
        sections.reverse()
        for item in sections:            
            indFn = self.dockWidgetContents.AddTelnetBt(item.AddrCaption, item.Addr)
            dri = _Dri(item.Addr, self.__DriChangeState, indFn)
            self.__dri.append(dri)
        sections.reverse()                                                       
             
        
    def __SetupGui(self, 
                   img : QtGui.QPixmap,
                   sections : [Item], 
                   icons : {str : QtGui.QPixmap}):
    
        self.setWindowTitle("IpsiusMonitor")
        icon = QtGui.QIcon()                 
        icon.addPixmap(img, QtGui.QIcon.Normal, QtGui.QIcon.On) 
        self.setWindowIcon(icon)                  
        
        self.resize(600, 400) # start size
        
        #Add central widget
        centralWd = QtGui.QWidget(self)        
        self.setCentralWidget(centralWd)
        centralWd.setLayout(QtGui.QVBoxLayout()) # is equal QtGui.QVBoxLayout(self.centralWd)
        centralWd.layout().setSpacing(0)        
        centralWd.layout().setContentsMargins(2, 2, 2, 2)   
        
        #Add dock widget
        dock = QtGui.QDockWidget(self)        
        dock.setFeatures(QtGui.QDockWidget.NoDockWidgetFeatures)
        dock.setAllowedAreas(QtCore.Qt.NoDockWidgetArea)      
        
        def ForAllSection(fn): 
            for item in sections:
                for s in item.Sections: fn(s)        
            
        #Add dock content with control OnXxx-processors and store reference        
        def OnRefreshAll():  
            ForAllSection(lambda sect : sect.OnRefresh())                                                                      
                
        def OnExpandAll(exp : bool):
            ForAllSection(lambda sect : sect.OnExpand(exp))            
                
        def CopyAll():
            text = []
            separator = '\n\n---------------------------\n\n'            
            ForAllSection(lambda sect : text.append( sect.GetText()) )                                                            
            QtGui.QApplication.clipboard().setText( separator.join(text) )
        
        self.dockWidgetContents = _DockContent(icons, OnExpandAll, OnRefreshAll, CopyAll)        
        dock.setWidget(self.dockWidgetContents)
        self.addDockWidget(QtCore.Qt.DockWidgetArea(4), dock)# 8 - top
        
        #Add scroll at central widget
        scroll = QtGui.QScrollArea(centralWd) # put scroll into centralWd
        scroll.setWidgetResizable(True)        
        centralWd.layout().addWidget(scroll) # bind scroll to centralWd layout 
        
        #Add scroll content and set layout
        scrollAreaWidgetContents = QtGui.QWidget(scroll)        
        scroll.setWidget(scrollAreaWidgetContents)
                
        scrollAreaWidgetContents.setLayout(QtGui.QVBoxLayout())
        scrollAreaWidgetContents.layout().setSpacing(0)        
        scrollAreaWidgetContents.layout().setContentsMargins(0, 0, 0, 0)   
       
        #Add section to scroll content     
        ForAllSection( lambda sect: scrollAreaWidgetContents.layout().addWidget(sect) )                                                      
        
        scrollAreaWidgetContents.layout().addStretch(700)
        
        #Add Status bar
        status = _StatusBar()        
        self.setStatusBar(status)
        ForAllSection( lambda sect: sect.BindStatusBar(status)  )                                
            
    def __DriChangeState(self, telnet : TelnetParams, process): # if process == None disconnect         
        
        descr = 'Connected to {0}:{1}'.format(telnet.Host, telnet.Port) if process != None \
        else 'No connection to {0}:{1}. TryConnect.'.format(telnet.Host, telnet.Port)        
        
        for item in self.__sects:
            if item.Addr == telnet:
                for s in item.Sections: s.Enable(process) 
         
        self.dockWidgetContents.textOutFn(descr)            
    
    
# ---------------------------------------------------
    
if __name__ == "__main__":    
    pass
        
        
            
                        

        
        
        