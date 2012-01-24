'''
Created on Mar 18, 2010

@author: root
'''

from CDUtilsPack.MetaUtils import Property
from PackUtils.Misc import Unquote
from CDUtilsPack.MiscUtils import AssertForException


# -------------------------------------------------------------

class CoreDeviceBroadcastInfo:
    """"
    Parsed device description. Device unique name is generated 
    using HwId and HwNumder.
    """
    
    def __init__(self, rawDesc : str):
        assert rawDesc
        self.staticName = ""
        self.name = ""
        self.params = {}
        
        def Add(name : str, val  = '<empty>'):
            assert not name in self.params.keys() 
            self.params[name] = val
        
        Add('HwNumber')
        Add('HwID')
        Add('IP')
        Add('MAC')
        Add('BooterPort')
        Add('CmpPort')
        Add('BootRelease')
        Add('BootReleaseInfo')
        Add('SoftRelease')
        Add('SoftReleaseInfo')        
        Add('StateDesc')
        Add('Owner')
        Add('FirstRecv')
        Add('LastRecv') 
        Add('DiscoveredCount')
        Add('BroadcastCounter')
        Add('StateEnter')
        
        self.__Parse(rawDesc)
        
        self.staticName = "Board_{0}_{1}".format(self["HwID"], self["HwNumber"])
        self.name = self.staticName
    
    
    def __iter__(self):
        for item in self.params:
            yield(item)
    
    
    def __getitem__(self, key : str) -> str:
        assert key in self.params
        return self.params[key]
    
    
    def __Set(self, name : str, val : str):
        self.params[name] = val;
    
    
    def __Parse(self, rawDesc : str):
        # rawDesc = "name = val; name = "val"; name = "val;val";"
        nameValList = [] # [(str, str)]
        
        CDevParamsSep = ';'
        CDevNameParamSep = '='
        
        def Add(list : [], item : str):
            item = item.strip()
            if not item: 
                return  
            nameVal = item.split(CDevNameParamSep)
            assert len(nameVal) == 2      
            name = nameVal[0].strip()
            val = Unquote(nameVal[1].strip())
            list += [(name, val)]
        
        insideQuotes = False
        currItem = ""
        for ch in rawDesc:
            if ch == '"' : 
                insideQuotes = not insideQuotes
            if ch == CDevParamsSep and not insideQuotes:
                Add(nameValList, currItem)
                currItem = ""
                continue
            currItem += ch
        Add(nameValList, currItem) # last
        
        # write to dictionary
        assert(len(nameValList) == len(self.params))
        
        for item in nameValList:
            self.__Set(item[0], item[1])
    
    
    # other : CoreDeviceBroadcastInfo
    def IsSameName(self, other) -> bool:
        return self.StaticName == other.StaticName
    
    
    # other : CoreDeviceBroadcastInfo
    def UpdateFrom(self, other):
        """Update from other CoreDeviceBroadcastInfo with the same name."""
        assert self.IsSameName(other)
        for key in self.params.keys():
            self.__Set(key, other[key])        
    
    
    def MainDesc(self, devType : str, sep = ',') -> str:
        return "HwNumber: {0}{4} HwID: {1} ({2}){4} IP: {3}".\
                format(self['HwNumber'], self['HwID'], devType, self['IP'], sep)
    
    
    def __Desc(self, sep : str, ignore = []) -> str:
        res = ""
        for key in self.params.keys():
            if key in ignore: continue
            res += "{0}: {1}{2}".format(key, self[key], sep)
        return res
    
    
    def AdvancedDesc(self, sep = ',') -> str:
        return self.__Desc(sep, ['HwNumber', 'HwID', 'IP'])
    
    
    def FullDesc(self, sep = ',') -> str:
        return self.__Desc(sep)
        
    
    Name = Property("name", True)   
    StaticName = Property("staticName")
    

# -------------------------------------------------------------

class CoreDeviceBroadcastInfoList:
    """
    Parsed list of device descriptions got from HwFinder.
    """
    
    def __init__(self, rawData : str):
        self.devList = [] # [CoreDeviceBroadcastInfo]
        
        # parse
        # rawList = "devDesc\n devDesc\n devDesc\n"
        if not rawData: return        
        lines = rawData.splitlines()
        self.devList = []
        for line in lines:
            line = line.strip()            
            if line:
                self.devList.append(CoreDeviceBroadcastInfo(line))        
    
    
    def __iter__(self):
        for item in self.devList:
            yield(item)
    
    
    def __len__(self) -> int:
        return len(self.devList)
        
    
# -------------------------------------------------------------        
# Tests
# -------------------------------------------------------------

def CoreDeviceBroadcastInfoGen(hwID : str, hwNumber : str) -> CoreDeviceBroadcastInfo:
    raw = 'HwNumber = {0}; HwID = {1}; IP = "127.0.0.1"; MAC = "00:00:00:00:00";\
           BooterPort = 0; CmpPort = 0; BootRelease = 0; \
           BootReleaseInfo = ""; SoftRelease = 0; \
           SoftReleaseInfo = ""; \
           StateDesc = "state"; Owner = "";\
           FirstRecv = "02.03.2010 12:30"; LastRecv = "02.03.2010 18:25"; \
           DiscoveredCount = 7; BroadcastCounter = 15; \
           StateEnter = "02.03.2010 10:10"'.format(hwNumber, hwID)
           
    return CoreDeviceBroadcastInfo(raw)

# -------------------------------------------------------------

def _CoreDeviceBroadcastInfoListGen() -> CoreDeviceBroadcastInfoList:
    raw = 'HwNumber = 1; HwID = 2; IP = "127.0.0.1"; MAC = "02:02:02:02:02";\
           BooterPort = 3; CmpPort = 4; BootRelease = 5; \
           BootReleaseInfo = "Boot Release Info1"; SoftRelease = 6; \
           SoftReleaseInfo = "Soft Release Info1"; \
           StateDesc = "someState desc1"; Owner = "board owner1";\
           FirstRecv = "01.02.2010 12:30"; LastRecv = "07.08.2010 18:25"; \
           DiscoveredCount = 1458; BroadcastCounter = 158; \
           StateEnter = "01.02.2010 10:10"\
           \n \
           \n \
           HwNumber = 2; HwID = 22; IP = "127.0.0.2"; MAC = "22:22:22:22:22";\
           BooterPort = 23; CmpPort = 24; BootRelease = 25; \
           BootReleaseInfo = "Boot Release Info2"; SoftRelease = 26; \
           SoftReleaseInfo = "Soft Release Info2"; \
           StateDesc = "someState desc2"; Owner = "board owner2";\
           FirstRecv = "07.08.2010 9:28"; LastRecv = "07.08.2010 18:30"; \
           DiscoveredCount = 15; BroadcastCounter = 8; \
           StateEnter = "07.08.2010 7:10"'
    
    return CoreDeviceBroadcastInfoList(raw)
    
# -------------------------------------------------------------

def ValidCoreDeviceBroadcastInfoTest(outputFn):
    outputFn("ValidCoreDeviceBroadcastInfoTest started ...")
    
    outputFn('Check: constructor')    
    raw = 'HwNumber = 1; HwID = 2; IP = "127.0.0.1"; MAC = "02:02:02:02:02";\
           BooterPort = 3; CmpPort = 4; BootRelease = 5; \
           BootReleaseInfo = "Boot Release Info"; SoftRelease = 6; \
           SoftReleaseInfo = "Soft; Release; Info"; \
           StateDesc = "someState desc"; Owner = "board owner";\
           FirstRecv = "01.02.2010 12:30"; LastRecv = "01.02.2010 18:25"; \
           DiscoveredCount = 7; BroadcastCounter = 8; \
           StateEnter = "01.02.2010 10:10";;;'
           
    desc = CoreDeviceBroadcastInfo(raw)
    
    outputFn("Name: " + desc.Name)
    assert desc.Name == "Board_2_1"
    assert desc.StaticName == "Board_2_1"
    
    desc.Name = "customName"
    assert desc.Name == "customName"
    assert desc.StaticName == "Board_2_1"
    
    desc.Name = desc.StaticName
    
    assert desc.MainDesc("DeviceSample")
    assert desc.AdvancedDesc()
        
    outputFn("CmpPort: " + desc["CmpPort"])
    assert desc["CmpPort"] == "4"
    
    outputFn("DiscoveredCount: " + desc["DiscoveredCount"])
    assert desc["DiscoveredCount"] == "7"
    
    outputFn('Check: is same')
    descNew = CoreDeviceBroadcastInfoGen(2, 1)
    outputFn("Name: " + descNew.Name)    
    assert descNew.IsSameName(desc)
    
    assert descNew.MainDesc("DeviceSample")
    assert descNew.AdvancedDesc()
    
    outputFn("DiscoveredCount: " + desc["DiscoveredCount"])
    assert descNew["DiscoveredCount"] == "7"
    
    outputFn('Check: update')
    desc.UpdateFrom(descNew)
    assert (descNew.AdvancedDesc() == desc.AdvancedDesc())
    
    outputFn('Check: two same parameters names')
    raw = 'HwNumber = 1; HwNumber = 2; IP = "127.0.0.1"; MAC = "02:02:02:02:02";\
           BooterPort = 3; CmpPort = 4; BootRelease = 5; \
           BootReleaseInfo = "Boot Release Info1"; SoftRelease = 6; \
           SoftReleaseInfo = "Soft Release Info1"; \
           StateDesc = "someState desc1"; Owner = "board owner1";\
           FirstRecv = "01.02.2010 12:30"; LastRecv = "07.08.2010 18:25"; \
           DiscoveredCount = 1458; BroadcastCounter = 158; \
           StateEnter = "01.02.2010 10:10'
    desc = CoreDeviceBroadcastInfo(raw)
    assert desc["HwNumber"] == "2"
    
    outputFn("ValidCoreDeviceBroadcastInfoTest: OK")

# -------------------------------------------------------------

def InvalidCoreDeviceBroadcastInfoTest(outputFn):
    outputFn("InvalidCoreDeviceBroadcastInfoTest started ...")
    
    def WithAssert(rawDesc : str):
        fn = lambda: CoreDeviceBroadcastInfo(rawDesc)
        AssertForException(fn, AssertionError)
    
    def WithoutAssert(rawDesc : str):
        desc = CoreDeviceBroadcastInfo(rawDesc)
    
    # missing name-val separator    
    WithAssert('HwNumber  1')
    WithAssert('HwNumber')
    WithAssert('_unknown_param_')
    WithAssert('=')
    WithAssert('= val')
    # invalid param count 
    WithAssert('HwNumber = 1;')
        
    outputFn("InvalidCoreDeviceBroadcastInfoTest: OK")

# -------------------------------------------------------------

def CoreDeviceBroadcastInfoListTest(outputFn):
    outputFn("CoreDeviceBroadcastInfoListTest started ...")
    
    outputFn('Check : empty')
    dl = CoreDeviceBroadcastInfoList("")
    outputFn("List size: {0}".format(len(dl)))
    assert len(dl) == 0
    
    outputFn("Check: list with 2 items")
    dl = _CoreDeviceBroadcastInfoListGen()
    outputFn("List size: {0}".format(len(dl)))
    assert len(dl) == 2
    
    for dev in dl:
        outputFn("{0}: {1}, {2}".format(dev.Name, dev.MainDesc("DeviceSample"),
                                  dev.AdvancedDesc()))
    
    outputFn("CoreDeviceBroadcastInfoListTest: OK")
    
# -------------------------------------------------------------

from TestDecor import UtTest
from AllTest.TestRunner import GTestRunner 


@UtTest
def CoreDeviceBroadcastInfoTest():
    assert CoreDeviceBroadcastInfoGen(0, 1)
    
    def Impl(outputFn):
        ValidCoreDeviceBroadcastInfoTest(outputFn)
        InvalidCoreDeviceBroadcastInfoTest(outputFn)
        CoreDeviceBroadcastInfoListTest(outputFn)
    
    GTestRunner.Run(Impl, traceTest = False)

# -------------------------------------------------------------

if __name__ == "__main__":
    import unittest
    unittest.main()
    
    
        