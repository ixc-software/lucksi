'''
Created on Jul 12, 2010

@author: root
'''
from CDUtilsPack.MetaUtils import Property

class TelnetParams:
    def __init__(self, host : str, port : int, 
                 login : str, password : str):
        self.host = host
        self.port = port
        self.login = login
        self.password = password        
    
    Host     = Property("host", True)
    Port     = Property("port", True)
    Login    = Property("login", True)
    Password = Property("password", True)
    
if __name__ == '__main__':
    p = TelnetParams("h", '127.0.0.1', '1', 'test')

