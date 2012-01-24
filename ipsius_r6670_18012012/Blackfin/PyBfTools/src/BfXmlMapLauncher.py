'''
Created on 07.11.2009

@author: Alexx
'''

import sys
import os.path
import io
import pickle
import struct
import time

import BfXmlMap
from CDUtils import QuickFileCache

# import getopt
# from optparse import OptionParser


# -----------------------------------------------------------------

'''
    
    - compiler no inline; linker individual map - ?!    
    
'''

# -----------------------------------------------------------------

def _DebugRun():
    CFileName = r'd:\proj\Ipsius\Blackfin\BFLoader\Release\bf537loader.map.xml' #  r'e:\proj\Ipsius\Blackfin\BFLoader\Release\bf537loader.map.xml'
    CLinesToProcess = 0 # 8 * 1024
    
    items = BfXmlMap.XmlMap(CFileName, CLinesToProcess).List
    
    sections = items.Sections
        
    print(items)
    print('')
    print(sections)
    

    CModule = r'.\Release\MfBfServerHost.doj' # r'.\Release\MfServer.doj'    
    moduleInfo = items.Module(CModule).FullInfo()
    
    print('')
    # print(moduleInfo)

# -----------------------------------------------------------------

def _SaveCache(cacheFileName : str, info : QuickFileCache, i : BfXmlMap.ModuleList):
    infoPickle = pickle.dumps(info)
    infoSize = struct.pack('i', len(infoPickle))
    dataPickle = pickle.dumps(i)
    
    with io.open(cacheFileName, 'w+b') as f:
        f.write(infoSize)
        f.write(infoPickle)
        f.write(dataPickle)
        
def _LoadCache(cacheFileName : str, info : QuickFileCache) -> BfXmlMap.ModuleList:
    
    with io.open(cacheFileName, 'r+b') as f:
        infoSize = struct.unpack('i', f.read(4))[0]   # size of int
        infoPickle = f.read(infoSize)
        cachedInfo = pickle.loads(infoPickle)
        
        if not cachedInfo.Equal(info): return None
        
        i = pickle.loads( f.read() )
        
        print('Cached,', info.TimeAsString)
        
        return i
            

def _GetModulesList(fileName) -> BfXmlMap.ModuleList:
    
    cacheFileName = fileName + '.cache'
                
    info = QuickFileCache(fileName)
                
    # cache load 
    if (os.path.exists(cacheFileName)):
        i = _LoadCache(cacheFileName, info)
        if i is not None: return i
    
    # get
    i = BfXmlMap.XmlMap(fileName, 0).List
    
    # save cache
    print('Cache saved,', info.TimeAsString)
    _SaveCache(cacheFileName, info, i)
    
    return i

# -----------------------------------------------------------------

if __name__ == '__main__':
    
    CCmdLineHelp = """Usage: [-ml] [-sl] [-mi <module name>] <xml file name>
        -ml for module list 
        -sl for sections list 
        -mi list full info for <module name> (can use multiple times)"""
        
    if len(sys.argv) < 2: 
        print(CCmdLineHelp)
        exit()
    
    # parse
    moduleList = False
    sectionList = False
    modules = []
    xmlFile = ''
    
    i = 1
    while i < len(sys.argv):        
        s = sys.argv[i]
        
        if s == '-ml': moduleList = True
        elif s == '-sl': sectionList = True
        elif s == '-mi': 
            modules.append(sys.argv[i+1])
            i += 1
        else:
            if s[0:1] == '-': raise Exception('Bad option ' + s)
            if len(xmlFile) != 0:  raise Exception('Xml already set! ' + s)
            xmlFile = s
                                
        i += 1
        
    if xmlFile == '': raise Exception('No xml file!')

    # do actions
    list = _GetModulesList(xmlFile)
    
    if moduleList:
        print("Modules:\n", list, "\n", sep='')
        
    if sectionList:
        print("Sections:\n", list.Sections, "\n", sep='')
        
    if (len(modules) > 0):
        for name in modules:
            m = list.Module(name)
            if m is None: raise Exception('Module {0} not found'.format(name))
            print(m.FullInfo(), "\n", sep='')
    

# -----------------------------------------------------------------
        
#    parser = OptionParser()
#    parser.add_option("-m", action="store_true", dest="ModuleList")
#    parser.add_option("-s", action="store_true", dest="SectionList")
#    parser.add_option("-d", action="store", type="string", dest="Module")
#
#    s = '-m -s -d adoj "aaaxml"'.split(' ')        
#    options, args = parser.parse_args()
#
#    print(s)
#    print(options)
#    print(args)
    
    
    
