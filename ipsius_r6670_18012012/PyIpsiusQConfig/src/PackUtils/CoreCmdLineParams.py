'''
Created on Mar 31, 2010

@author: root
'''

import os
from CDUtils import CmdOptions, GetScriptDir, UserException

# ------------------------------------------------------------

class CoreCmdLineParams:
    """Parsed command line parameters."""
    
    def __init__(self, cmdLine : [str]):
        cmdLine = cmdLine[1:len(cmdLine)]

        def Add(opts : CmdOptions, names : str or (str), desc : str, 
                rec = True, varType = "str"):
            param = ("{0}:{1}".format(names, varType), ) if varType else ()
            opts.Add(names, desc, param, rec)
        
        opts = CmdOptions(autoAddShortNames = True)    
        Add(opts, "exeFile", "Path to .exe file with the file name included.")
        Add(opts, "confDir", "Directory to store generated .conf files.", False)
        Add(opts, "fwDir", "Firmware directory.", False)
        Add(opts, ("checkDeviceDesc", "cdd"),
            ("Check description of all devices in folder 'Devices' "
             "before starting main application."), False, None)
        Add(opts, ("traceToGui", "tg"), 
            ("Show trace info in 'Log' and 'ProjIpsius Output' widgets on main "
             "window."), False, None)
        Add(opts, ("traceToFile", "tf"),
            ("Enable saving trace info to file, "
             "option's parameter means how many records "
             "will be saved at once (0, -1 - all records" 
             "will be saved just on close)."), False, "int")
        Add(opts, ("udpLogViewFile", "l"), 
            "Path to UdpLogViewer.exe file with the file name included.", False)
        Add(opts, ("testMode", "t"),
            ("Run all tests. Don't start main application."), False, None)
        
        self.sett = opts.Parse( cmdLine, exitOnHelp = True)
        
        self.__CheckFile('exeFile')
        if self.RunUdpLogView: self.__CheckFile('udpLogViewFile')
        self.__CheckDir('confDir', self.__ExeRelativeDir("../config"))
        self.__CheckDir('fwDir', self.__ExeRelativeDir("../firmware"))
    
    
    def __CheckFile(self, settName : str):
        """
        Check that setting 'settName' is file path, try make file path 
        absolute (from script directory). Can raise exception.
        """
        res = self.sett[settName] 
        if not os.path.isabs(res):
            res = GetScriptDir(res)
        if not os.path.exists(res):
            raise UserException("File '{}' was not found".format(res))
        if not os.path.isfile(res):
            raise UserException("'{}' is not a file".format(res))
        self.sett[settName] = res
    
        
    def __CheckDir(self, settName : str, defaultDir : str):
        """
        If not 'settName' in settings, make default directory 'defaultDir'
        and save it in settings. Check that setting is directory,
        make it path absolute (from script directory) and create it if need. 
        """
        if not settName in self.sett.keys():
            self.sett[settName] = defaultDir                
        
        res = self.sett[settName]
        if not os.path.isabs(res):
            res = GetScriptDir(res)
        if not os.path.exists(res):
            os.makedirs(res)
        elif not os.path.isdir(res):
            raise UserException("'{}' is not a directory".format(res))
        self.sett[settName] = res
    
    
    def __ExeRelativeDir(self, path : str) -> str:
        """Create directory and return absolute path."""
        assert self.ExeFile
        exeDir = os.path.dirname(self.ExeFile)
        res = os.path.join(exeDir, path)
        return os.path.normpath(res)    
    
        
    @property
    def ExeFile(self) -> str:
        return self.sett["exeFile"]
    
    
    @property
    def ConfigDir(self) -> str:
        return self.sett["confDir"]
    
    
    @property
    def FirmwareDir(self) -> str:
        return self.sett["fwDir"]
    
    
    @property
    def RunUdpLogView(self) -> bool:
        return "udpLogViewFile" in self.sett
    
    
    @property
    def UdpLogViewFile(self) -> str:
        assert self.RunUdpLogView
        return self.sett["udpLogViewFile"]
    
    
    @property
    def TraceToGui(self) -> bool:
        return "traceToGui" in self.sett
        
        
    @property
    def TraceToFile(self) -> bool:
        return "traceToFile" in self.sett
    
    
    @property
    def TraceToFileCount(self) -> int:
        assert self.TraceToFile
        return self.sett["traceToFile"]    
    
    
    @property
    def CheckDeviceDesc(self) -> bool:
        return "checkDeviceDesc" in self.sett
    
    
    @property
    def TestMode(self) -> bool:
        return "testMode" in self.sett
    
    
# ------------------------------------------------------------
# Tests
# ------------------------------------------------------------

from TestDecor import UtTest
from AllTest.TestRunner import GTestRunner
from PackUtils.CorePathes import TempFiles
import sys

@UtTest
def CoreCmdLineParamsTest():
    
    def Check(argv : [str], exeFile : str, confDir : str, fwDir : str, 
             traceToGui = False, checkDesc = False, traceToFileCount = None,
             logFile  = None, testMode = False):
        c = CoreCmdLineParams(argv)
        assert c.ExeFile == exeFile
        assert c.ConfigDir == confDir
        assert c.TraceToGui == traceToGui
        assert c.CheckDeviceDesc == checkDesc
        if traceToFileCount != None:
            assert c.TraceToFile
            assert c.TraceToFileCount == traceToFileCount
        else:
            assert not c.TraceToFile
        if logFile != None:
            assert c.UdpLogViewFile == logFile
            assert c.RunUdpLogView
        else:
            assert not c.RunUdpLogView
        assert c.TestMode == testMode
    
    def Impl(outputFn):
        file = sys.argv[0] 
        if not os.path.isabs(file):
            file = os.path.abspath(file)
        confDir = TempFiles.TestFile("_test_configdir")
        fwDir = TempFiles.TestFile("_test_fwdir")
        
        # test with all params
        argv = ["self", 
                "/exeFile", file, 
                "/confDir", confDir, 
                "/fwDir", fwDir,
                "/traceToFile", -1,
                "/traceToGui",
                "/cdd",
                "/l", file,
                "/testMode"]
        Check(argv, exeFile = file, confDir = confDir, fwDir = fwDir, 
              traceToGui = True, checkDesc = True, traceToFileCount = -1, 
              logFile = file, testMode = True)
         
        # test with auto generated dirs
        argv = ["self", 
                "/exeFile", file,
                "/tf", 5]
        Check(argv, file, GetScriptDir("../config"), 
              GetScriptDir("../firmware"), traceToFileCount = 5)
        
        os.removedirs(confDir)
        os.removedirs(fwDir)
        
        outputFn('CoreCmdLineParamsTest: OK')   
    
    GTestRunner.Run(Impl, traceTest = False) 
    
# ------------------------------------------------------------

if __name__ == "__main__":
    
    import unittest
    unittest.main()
    