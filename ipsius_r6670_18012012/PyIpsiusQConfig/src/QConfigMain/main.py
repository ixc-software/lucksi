'''
Created on Feb 9, 2010

@author: root
'''


def Run():
    """
    Add ./src path to PYTHONPATH. Parse command line arguments and run tests
    if need (each test create QtGui.QApplication or QtCore.QCoreApplication, 
    so we can't run tests from GuiApplication class, which is inherited from 
    QtCore.QCoreApplication). Start main application.
    """
    
    from MainPreRunSetup import PreRunSetup
    PreRunSetup(True)    
    
    import sys
    from CDUtilsPack.MetaUtils import UserException
    from PackUtils.CoreBaseException import CoreBaseException
    from QConfigMain.GuiApplication import GuiApplication
    from PackUtils.CoreCmdLineParams import CoreCmdLineParams
    from AllTest.TestRunner import TestRunnerErr
    
    try:
        sett = CoreCmdLineParams(sys.argv) # can raise UserException
        
        # run in test mode
        if sett.TestMode:
            from AllTest.RunAllTests import RunAllTests
            RunAllTests()
            return
        
        # run tests to check all devices types
        if sett.CheckDeviceDesc:
            from QConfigMain.GuiDeviceFactory import DevicesTypeCheck
            DevicesTypeCheck(sett.ExeFile) 
            
        # run application    
        app = GuiApplication(sett)
    
    except UserException as e:
        print ("Aborted!\nStart-up error:", e)
    
    except TestRunnerErr as e:
        print("Aborted!\n{0}: {1}".format(type(e).__name__, e))
    
    except (CoreBaseException, Exception) as e:
        print("Aborted!\n{0}: {1}".format(type(e).__name__, e))
        
    # Show other exceptions with traceback 

# ------------------------------------------------------------

if __name__ == "__main__":
    
    Run()
