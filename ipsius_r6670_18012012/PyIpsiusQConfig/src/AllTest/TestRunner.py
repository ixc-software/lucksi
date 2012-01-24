'''
Created on Jul 16, 2010

@author: root
'''

from PyQt4 import QtGui
import traceback
import sys
from QConfigMain.MainPreRunSetup import PreRunSetup
from CDUtilsPack.MetaUtils import Property
import os

# ------------------------------------------------------

class TestRunnerErr(Exception):
    pass

# ------------------------------------------------------

class TestRunner:
    """
    In any run method:
        'outputFn' - fn(text : str) -> None
    
    Common usage:
        @UtTest
        def Test():
            def TestFn(<list of test parameter>): # for description see needed run method info
                pass
    
        GTestRunner.RunX(Impl, <list of parameters>)
        
    RunX - one of the run methods of TestRunner.    
    """
    
    def __init__(self):
        self.app = None # QtGui.QApplication
        self.defaultHook = sys.excepthook
        self.testFailed = False
        self.ipsiusExePath = None
        self.udpLogExePath = None
        
        # using to close inner runner on error
        self.runnerImpl = None # TestIpsiusRunner or other runner class
        
        def CustomHook(etype, value, tb): 
            """
            Using to catch all exception in tests. Inside Qt event loop also.
            """
            traceback.print_exception(etype, value, tb)
            self.testFailed = True
            
            if self.runnerImpl:
                self.runnerImpl.Close(1)
                
            if self.app: 
                self.app.exit(1) # goto code after app.exec_()
                self.app = None
        
        sys.excepthook = CustomHook
        
        self.__LoadRunnerParams()
        
    
    def __LoadRunnerParams(self):
        from CDUtilsPack.MiscUtils import LoadModuleFromFile
        from PackUtils.CorePathes import TestFiles
        
        if not os.path.exists(TestFiles.ConfigFile):
            raise TestRunnerErr("Please create module " + TestFiles.ConfigFile + " from "
                                 "'_TestConst.py' and customize parameters.")
        
        def CheckPath(val : str):
            if os.path.exists(val): return
            raise TestRunnerErr("In file '{0}': path '{1}' doesn't exist.".
                                format(TestFiles.ConfigFile, val))
        
        module = LoadModuleFromFile(TestFiles.ConfigFile)
        self.ipsiusExePath = module.CTestIpsiusPath
        CheckPath(self.ipsiusExePath)
        self.udpLogExePath = module.CTestLogPath            
        CheckPath(self.udpLogExePath)
        
    
    def __Exit(self, exitCode : int):
        self.runnerImpl = None
        self.app.exit(exitCode)
    
    
    def __Output(self, traceTest : bool, text : str):
        if not traceTest : return 
        print(text)
    
        
    def __BeforeRun(self, preRunSetupWithUi : bool):
        PreRunSetup(preRunSetupWithUi)
        assert not self.runnerImpl
        assert not self.app
    
    
    def __CheckTestEnd(self):
        self.app = None
        
        sys.excepthook = self.defaultHook # reset to default
        if self.testFailed: raise TestRunnerErr("Test failed!")
        
    
    def Run(self, testFn, traceTest = False, preRunSetupWithUi = True):
        """
        'testFn' - fn(outputFn) -> None
        """
        self.__BeforeRun(preRunSetupWithUi)
        outputFn = lambda text: self.__Output(traceTest, text) 
        testFn(outputFn)
    
    
    def RunApp(self, testFn, traceTest = False, 
               startEventLoop = False):
        """
        'testFn'  - fn(params : TestParams) -> None,
        """
        self.__BeforeRun(preRunSetupWithUi = True)
        
        from PackUtils.Misc import CheckFnArgs
        from AllTest.TestParams import TestParams
        
        CheckFnArgs(testFn, (TestParams,), None)
        assert isinstance(traceTest, bool)
        assert isinstance(startEventLoop, bool)
            
        self.app = QtGui.QApplication(sys.argv)
       
        testFn(TestParams(lambda exitCode: self.__Exit(exitCode), 
                          lambda text: self.__Output(traceTest, text)))
        
        if startEventLoop: 
            self.testFailed = True if self.app.exec_() != 0 else False
        
        # come here after app.exit() is called
        self.__CheckTestEnd()
    
    
    def RunIpsius(self, testFn, traceTest = False, startEventLoop = False, withDri = False, enableDriTrace = False):
        """
        def TestFn(p : TestParams):
            p.Output("test started")
            ...
            p.Complete(exitCode = 0)
    
        or
            
        def Impl(p : TestParams):
            
            class Test:
                def FirstCallBack():
                    p.Output("test started")
                    
                def ErrorCallback(err : str):
                    p.Output(err)
                    p.Complete(exitCode = 1)
                
                def LastCallback():
                    p.Complete(exitCode = 0)
                    
            t = Test()
        """
        self.__BeforeRun(preRunSetupWithUi = True)
        
        from PackUtils.Misc import CheckFnArgs
        from AllTest.TestParams import TestParams
        from DRIDomainTest.TestIpsiusRunner import TestIpsiusRunner
        
        assert isinstance(traceTest, bool) 
        CheckFnArgs(testFn, (TestParams, ), None)
        
        
        def TestFnWrapper(dummi : TestParams):
            self.runnerImpl = TestIpsiusRunner(testFn, 
                                 lambda text: self.__Output(traceTest, text), 
                                 lambda exitCode : self.__Exit(exitCode), withDri, enableDriTrace,
                                 dummi.CreateCoreTrace(traceTest), self.IpsiusPath, self.UdpLogPath)
        
        self.RunApp(TestFnWrapper, traceTest, startEventLoop)
    
    
    def RunGuiMainIpsiusDri(self, testFn, showWidget : bool, 
                            parentLeftWidgetType, parentRightWidgetType = None,
                            traceTest = False, enableDriTrace = True,
                            startEventLoop = False):
        """
        See TestRunnerIpsiusDri for parameters description.
                       
        def Impl(p : TestParams, parentWidget):            
            myClass = MyClass(parentWidget, ...)
            ...
            p.Complete()

        or
            
        def Impl(p : TestParams, parentLeftWidget, parentRightWidget):            
            myClass1 = MyClass1(parentLeftWidget, ...)
            myClass2 = MyClass2(parentRightWidget, ...)
            ...
            p.Complete()    
        """
        self.__BeforeRun(preRunSetupWithUi = True)
        
        from AllTest.TestParams import TestParams
        
        assert isinstance(showWidget, bool)
        assert issubclass(parentLeftWidgetType, QtGui.QWidget)
        if parentRightWidgetType != None: 
            issubclass(parentRightWidgetType, QtGui.QWidget)
        assert isinstance(traceTest, bool)  
        
        from QConfigMainTest.TestGuiMainIpsiusRunner import TestGuiMainIpsiusRunner 
        
        def TestFnWrapper(dummi : TestParams):
            self.runnerImpl = TestGuiMainIpsiusRunner(showWidget, parentLeftWidgetType, 
                                                      parentRightWidgetType)
            self.runnerImpl.Run(testFn, lambda text : self.__Output,
                                lambda exitCode: self.__Exit(exitCode), 
                                True, enableDriTrace,
                                dummi.CreateCoreTrace(traceTest),
                                self.IpsiusPath, self.UdpLogPath)
        
        self.RunApp(TestFnWrapper, traceTest, startEventLoop)  
    
    
    IpsiusPath = Property("ipsiusExePath")
    UdpLogPath = Property("udpLogExePath")

# ------------------------------------------------------

GTestRunner = TestRunner()

# ------------------------------------------------------



    
