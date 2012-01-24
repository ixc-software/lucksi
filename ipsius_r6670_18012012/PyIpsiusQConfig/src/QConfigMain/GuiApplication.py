'''
Created on Jul 5, 2010

@author: root
'''
from CDUtilsPack.Telnet.TelnetParams import TelnetParams
from PackUtils.CoreBaseException import CoreBaseException
import traceback

import sys
from PyQt4 import QtGui, QtCore
from CDUtilsPack.MetaUtils import UserException

from DRIDomain.TelnetClientDri import DriError
from QConfigMain.CoreFieldValidator import ValidatorErr
from QConfigMain.CoreFieldConverter import ConverterErr
from PackUtils.CoreTrace import CoreTrace
from QConfigMain.CoreLogger import CoreLogger, CoreLogRecordProcessor
from DRIDomain.CoreIpsiusRunner import CoreIpsiusRunner
from PackUtils.CoreCmdLineParams import CoreCmdLineParams
from QConfigMain.CoreLogStreams import PrintLogStream, FileLogStream
from QConfigMain.CoreUdpLogViewRunner import CoreUdpLogViewRunner
from QConfigMain.GuiMainWidget import GuiMainWidget
from QConfigMain.GuiDeviceConfig import GuiDeviceConfigErr
from QConfigMain.GuiDevice import DeviceNameError
from CDUtilsPack.MetaUtils import UserException


# ---------------------------------------------------------

class GuiApplication(QtGui.QApplication):
    """
    QConfigMain class. Using to start ProjIpsius. If ProjIpsius is started 
    without errors create main window.
    """
    
    def __init__(self, sett : CoreCmdLineParams):
        QtGui.QApplication.__init__(self, sys.argv)
        self.sett = sett
        self.log = None          # CoreLogger
        self.ipsius = None       # CoreIpsiusRunner
        self.udpLogViewer = None # CoreUdpLogViewRunner
        self.gui = None          # GuiMainWidget
        self.closed = False
        self.exitCode = 0
        
        self.aboutToQuit.connect(self.__OnExit)
        
        self.__InitLog()
        self.__StartIpsius()
        
        # start Qt event loop
        self.exitCode = self.exec_()
    
    
    def __InitLog(self):
        assert not self.log
        self.log = CoreLogger()
        RP = CoreLogRecordProcessor
        self.log.AddOutput(PrintLogStream(RP.CreateIncludeFilterFn([GuiApplication]), 
                           RP.CreateFormatFn("{text}")))
        if self.sett.TraceToFile:
            self.log.AddOutput(FileLogStream('GuiIpsiusQuickConfig', 
                                             self.sett.TraceToFileCount)) 
    
    
    def __Print(self, text : str):
        self.log.Add(self, text)    
    
    
    def __OnExit(self):
        assert not self.closed
        # if close log via termination, it won't save any data
        # if self.udpLogViewer:
        #     self.udpLogViewer.Close()
        #     self.udpLogViewer = None
        
        # try close gui (if it has unsaved changes and if wouldn't close)
        if self.gui: 
            if not self.gui.close(): 
                return 
            self.gui = None
        
        self.closed = True
        
        # close ipsius
        if self.ipsius:
            self.ipsius.Close()
            self.ipsius = None
        # close log
        self.log.Close()
            
        sys.exit(self.exitCode)
    
    
    def __MainWidgetErrorHandler(self, e : Exception):
        if self.closed:
            return 
        
        msgBoxErrors = [DriError, ValidatorErr, ConverterErr, 
                        GuiDeviceConfigErr, DeviceNameError]
        title = "DRI Error" if isinstance(e, DriError) else "Error"
        if type(e) in msgBoxErrors:
            Msg = QtGui.QMessageBox.warning
            Msg(self.gui, title, '{}'.format(e), QtGui.QMessageBox.Ok)
            return
        
        if not issubclass(type(e), CoreBaseException):
            traceback.print_exc(file=sys.stdout)
        else:
            self.__Print("Aborted!\n{0}: {1}\n".format(type(e).__name__, str(e)))
        
        self.__OnExit()
    
    
    def __CreateMainWindow(self):
        assert self.ipsius
        try:
            self.gui = GuiMainWidget(self.sett, self.log,
                                     self.ipsius.Params.DomainName,
                                     self.ipsius.Params.TelnetParams,
                                     self.__MainWidgetErrorHandler)
        except Exception as e:
            self.__MainWidgetErrorHandler(e) 
            
    
    def __StartIpsius(self):
        """
        Create UdpLogViewer and MainWindow on Started(). Is any error occur,  
        close application. Print startup output on screen.
        """
        
        def Started(port : int):
            self.__Print("ProjIpsius started on port {}.".format(port))
            self.ipsius.Params.TelnetParams.Port = port
            if self.sett.RunUdpLogView: 
                self.__StartGuiLog()
            self.__CreateMainWindow()
            
        def Error(msg : str):
            if self.closed: return
            self.__Print("Ipsius error: " + msg)
            if "QProcess.Crashed" in msg: return # wait finished() signal
            self.__OnExit()             
        
        def Output(text: str):
            self.__Print("ProjIpsius output: " + text)
            self.log.Add(self.ipsius, text)
        
        def Finished(exitCode : int, exitStatus : QtCore.QProcess.ExitStatus):
            if self.closed: return
            self.ipsius = None
            self.gui.IpsiusUnexpectedlyClosed = True
            self.__OnExit()
        
        self.__Print("Starting ProjIpsius ...")
        telnetParams = TelnetParams("127.0.0.1", 0, "1", "test")
        self.ipsius = CoreIpsiusRunner(Started, Finished, Error, Output,
                self.sett, telnetParams, CoreTrace(self.log.Add, True))
        self.ipsius.Run()
    
    
    def __StartGuiLog(self):
        
        def Started():
            self.__Print("UdpLogViewer started and listening to port: {}.".
                  format(self.ipsius.Params.UdpLogPort))
        
        def Error(msg : str):
            if self.closed: return
            self.__Print("UdpLogViewer error: " + msg)
            if "QProcess.Crashed" in msg:
                return # wait finished() signal
            # OnExit()     
        
        def Output(text : str):
            self.log.Add(self.udpLogViewer, text)
        
        def Finished(exitCode : int, exitStatus : QtCore.QProcess.ExitStatus):
            self.__Print("UdpLogViewer closed.")
            self.udpLogViewer = None                                
        
        assert self.ipsius        
        self.udpLogViewer = CoreUdpLogViewRunner(\
                        Started, Finished, Error, Output,
                        self.sett, CoreTrace(self.log.Add, True))
        self.udpLogViewer.AddPort(self.ipsius.Params.UdpLogPort)
        self.udpLogViewer.Run()
        
        