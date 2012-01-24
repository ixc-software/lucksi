'''
Created on Jul 12, 2010

@author: root
'''

from PyQt4 import QtCore
from PackUtils.CorePathes import TempFiles
from PackUtils.CoreTrace import CoreTrace
from PackUtils.Misc import ToStr, QProcessErrorToStr, CheckFnArgs
from CDUtilsPack.MetaUtils import Property

# -----------------------------------------------------

#class ICoreProcessRunnerEvents:
#    """
#    Base interface for CoreProcessRunner owner.
#    """
#    def Started(self): pass
#    def Finished(self, exitCode : int, 
#                 exitStatus : QtCore.QProcess.ExitStatus): pass
#    def Error(self, data : str): pass
#    def Output(self, data : str): pass

# -----------------------------------------------------

class CoreProcessRunner:
    """
    Using to run ProjIpsius as separate process. 
    """
    
    def __init__(self, onStartedFn, onFinishedFn, onErrorFn , onOutputFn, 
                 trace : CoreTrace):
        """
        'onStartFn'  - fn() -> None
        'onFinishFn' - fn(exitCode : int, exitStatus : QtCore.QProcess.ExitStatus)
        'onErrorFn'  - fn(data : str)
        'onOutputFn' - fn(data : str)
        """
        CheckFnArgs(onStartedFn, (), None)
        CheckFnArgs(onFinishedFn, (int, QtCore.QProcess.ExitStatus), None)
        CheckFnArgs(onErrorFn, (str,), None)
        CheckFnArgs(onOutputFn, (str,), None)
        self.trace = trace
        self.process = QtCore.QProcess()
        self.closed = False
        
        self.__ConnectSigSlots(onStartedFn, onFinishedFn, onErrorFn , onOutputFn)
    
    
    def __ConnectSigSlots(self, onStartedFn, onFinishedFn, 
                          onErrorFn , onOutputFn):
        
        def OutputHandler():
            data = ToStr(self.process.readAllStandardOutput().data())
            self.__Trace("OutputHandler: " + data)
            onOutputFn(data)
            
        def ErrorHandler():
            data = ToStr(self.process.readAllStandardError().data())
            self.__Trace("ErrorHandler: " + data)
            onErrorFn(data)
         
        def StartedHandler():
            self.__Trace("StartedHandler")
            onStartedFn()
            
        def FinishedHandler():
            self.__Trace("FinishedHandler")
            onFinishedFn(self.process.exitCode(), 
                                              self.process.exitStatus())            
        
        def QProcessErrorHandler(e : QtCore.QProcess.ProcessError):
            strErr = QProcessErrorToStr(e) 
            self.__Trace("QProcessErrorHandler: " + strErr)    
            onErrorFn(strErr)
        
        self.process.readyReadStandardOutput.connect(OutputHandler)
        self.process.readyReadStandardError.connect(ErrorHandler)
        self.process.finished.connect(FinishedHandler)
        self.process.started.connect(StartedHandler)
        self.process.error.connect(QProcessErrorHandler)                  

    
    def __del__(self):
        assert self.closed

        
    def __Trace(self, text : str):
        self.trace.Add(self, text)
    
    
    def __CheckNotClosed(self):
        assert not self.closed
    
    
    def Run(self, processName : str, args : []):
        """
        Starting ProjIpsius with list of command line parameters 'args'         
        """
        self.__CheckNotClosed()
        self.__Trace("Run with params: {}".format(args))
        self.process.start(processName, args)        
        
    
    def Close(self):
        """
        Using QProcess.terminate()
        """
        self.__CheckNotClosed()
        self.__Trace("Close")
        self.closed = True
        self.process.kill()
        if not self.process.waitForFinished(2000):
            self.process.kill()
            self.process.waitForFinished()
    
    
    IsClosed = Property("closed")

        