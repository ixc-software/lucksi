'''
Created on Feb 15, 2010

@author: root
'''

from DRIDomain.CoreIpsiusRunnerBase import (CoreIpsiusRunnerBase, 
                                       CoreIpsiusCmdLineParams)
from PackUtils.CoreCmdLineParams import CoreCmdLineParams
from PackUtils.CoreTrace import CoreTrace
from CDUtilsPack.Telnet.TelnetParams import TelnetParams


# -------------------------------------------------------

class CoreIpsiusRunner:
    """Using to run ProjIpsius."""
    
    def __init__(self, onStartedFn, onFinishedFn, onErrorFn , onOutputFn, 
                 sett : CoreCmdLineParams, telnetParams : TelnetParams,
                 trace : CoreTrace):
        """
        See CoreIpsiusRunnerBase description.
        """
        params = CoreIpsiusCmdLineParams(telnetParams, sett, True)
        self.impl = CoreIpsiusRunnerBase(onStartedFn, onFinishedFn, 
                                         onErrorFn , onOutputFn, params, trace)
        

    def Run(self):
        self.impl.Run()        
        
        
    def Close(self):
        self.impl.Close()
                
    @property
    def Params(self) -> CoreIpsiusCmdLineParams:
        return self.impl.Params



