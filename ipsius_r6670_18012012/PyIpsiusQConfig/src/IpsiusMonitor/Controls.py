'''
Created on 31.10.2010

@author: Valkiriy
'''

from PyQt4 import QtGui, QtCore
from CDUtilsPack import MiscUtils
from types import FunctionType          
        
class Panel(QtGui.QWidget):
    '''
        Common control panel gui
    '''
    __Size = 20    
    
    def __init__(self, name : str):
        super().__init__()
        l = QtGui.QHBoxLayout()
        l.setSpacing(0)        
        l.setContentsMargins(5, 0, 0, 0)  
        self.setLayout(l)
        l.addWidget(QtGui.QLabel(name))        
        #l.addStretch(10)                             
    
    def AddWidget(self, wd : QtGui.QWidget, fixed : bool, atBack = True, fn = None, sig = 'clicked()'):        
        wd.setMaximumHeight(self.__Size)
        
        if atBack: self.layout().addWidget(wd)
        else: self.layout().insertWidget(2, wd)
        
        if fixed:   wd.setFixedSize(self.__Size, self.__Size)
        else:       wd.setMinimumHeight(self.__Size)
                            
        if fn: self.connect(wd, QtCore.SIGNAL(sig), fn)  
        
#---------------------------------------------------------------------------- 
        
class Timer:
    '''
        Callback timer
    '''
    def __init__(self, period : int, onRefreshFn): #onRefreshFn : Fn() 
        self.__poll = QtCore.QTimer()          
        self.__poll.timeout.connect(onRefreshFn)
        self.__period = period        

    def Enable(self, enable : bool):
        if self.__poll.isActive() == enable: return
        if enable: self.__poll.start(self.__period)
        else: self.__poll.stop()  
        
#----------------------------------------------------------------------------           
        
class PanelButton(QtGui.QPushButton):
        
      
    def __init__(self, pixmap : QtGui.QPixmap = None, name : str = None, toolTip = None):        
        super().__init__()                
        self.setIcon(QtGui.QIcon())                                                  
        if name: self.setText(name)
        if pixmap: self.SetIcon(pixmap)
        self.setToolTip(toolTip)
        
    def SetIcon(self, pixmap : QtGui.QPixmap):
        assert(isinstance(pixmap, QtGui.QPixmap))
        icon = QtGui.QIcon()                 
        icon.addPixmap(pixmap, QtGui.QIcon.Normal, QtGui.QIcon.On)
        self.setIcon(icon)

#def SetIcon(bt : QtGui.QPushButton) -> QtGui.QPushButton:

#----------------------------------------------------------------------------  

class ElapsedTime(QtGui.QLabel):
    '''
        Label of elapsed time in minutes.
    '''
    def __init__(self):
        super().__init__()
        self.startTime = None                            
        self.refresh = Timer(1000 * 60, self.__Refresh)        
        
    def __Refresh(self):                             
        sec = self.startTime.secsTo(QtCore.QTime.currentTime())                
        if (sec > 3600):
            self.setText('> hour ago!')
            self.refresh.Enable(False)
            
        else: self.setText('{} m ago '.format(int(sec/60))) 
            
    def Reset(self):        
        self.startTime = QtCore.QTime.currentTime()
        self.refresh.Enable(True)        
        self.__Refresh()        
    
#----------------------------------------------------------------------------                       
        
        
if __name__ == '__main__': pass        
