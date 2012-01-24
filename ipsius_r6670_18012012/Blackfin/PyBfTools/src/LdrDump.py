'''
Created on 24.11.2009

@author: Alex

'''

import io
import struct
import sys
import os.path

from CDUtils import IntToHex, TestBit, SetBit, \
     DumpBytes, ListClassFields, StatElement, IntToSize, \
     SplitCommandLineByKeys, QuickFileCache, CreateLazyInitProxy

        
# ----------------------------------------------------------

_CBlockHeaderSize = 10
_CEofRAM = 512 * 1024 * 1024

_CZeroFlagBit = 0
_CCpuFlagBit  = 1

class Block:
    def __init__(self, data, offs):
        self.m_data = data
        self.m_offs = offs
        
        self.m_addr, self.m_count, self.m_flags = struct.unpack('IIH', data[offs:offs+_CBlockHeaderSize])
        
        self.m_fZeroFill        = TestBit(self.m_flags, _CZeroFlagBit)
        self.m_fCpuType         = TestBit(self.m_flags, _CCpuFlagBit)        
        self.m_fInit            = TestBit(self.m_flags, 3)
        self.m_fIgnore          = TestBit(self.m_flags, 4)
        self.m_fCompressed      = TestBit(self.m_flags, 13)        
        self.m_fLast            = TestBit(self.m_flags, 15)
        
        assert self.m_fCompressed == False, str(self)
        assert self.m_fCpuType == True, str(self)
        
    @property
    def Final(self) -> bool:
        return self.m_fLast
    
    @property
    def Ignored(self) -> bool:
        return self.m_fIgnore
    
    @property
    def ZeroFilled(self) -> bool:
        return self.m_fZeroFill
    
    def NextBlockOffset(self) -> int:
        size = _CBlockHeaderSize
        if not self.m_fZeroFill: size += self.m_count
        return self.m_offs + size
        
    def __BoolFlag(self, flagName : str) -> str:
        flagVal = getattr(self, flagName)
        if flagVal:
            return " " + flagName 
        return ""
    
    def __BoolFlags(self):
        return "{zero}{init}{ignore}{last}".format(
        zero = self.__BoolFlag("m_fZeroFill"),
        init = self.__BoolFlag("m_fInit"), 
        ignore = self.__BoolFlag("m_fIgnore"),        
        last = self.__BoolFlag("m_fLast") )
    
    def __str__(self):
        return "{offs}: Addr = {addr} Count = {count} Flags = {flags} {flagsStr}".format(
        offs = IntToHex(self.m_offs),
        addr = IntToHex(self.m_addr), 
        count = IntToSize(self.m_count, -10),
        flags = IntToHex(self.m_flags, 2),
        flagsStr = self.__BoolFlags() )
    
    @property     
    def Addr(self):
        return self.m_addr
    
    @property
    def EndAddr(self):
        return self.Addr + self.Size - 1
    
    @property
    def Size(self):
        return self.m_count
    
    def AddrInside(self, addr : int):
        return addr >= self.Addr and addr <= self.EndAddr
    
    def AddrToOffsetInFile(self, addr : int) -> int:
        assert self.AddrInside(addr)
        return self.m_offs + _CBlockHeaderSize + (addr - self.Addr)
    
    def Dump(self, addr, count : int) -> bytes:
        assert self.AddrInside(addr)
        assert self.AddrInside(addr + count - 1)
        
        if self.m_fZeroFill:
            return b'\x00' * count
        
        startAddr = self.m_offs + _CBlockHeaderSize + addr - self.Addr         
        return self.m_data[startAddr:startAddr + count]
    
    @property
    def Data(self) -> bytes:
        assert not self.m_fIgnore
        
        if self.m_fZeroFill: return b'\x00' * self.Size

        startAddr = self.m_offs + _CBlockHeaderSize
        res = self.m_data[startAddr:startAddr + self.Size]
        assert len(res) == self.Size        
        return res
    
    def NonRam(self):
        return self.Ignored or self.Addr >= _CEofRAM
    
    def FullBinaryBody(self) -> bytes:
        return self.m_data[self.m_offs:self.NextBlockOffset()]

# ----------------------------------------------------------

class BlocksStats:
    def __init__(self, blockList : [Block]):
        
        prevRamBlock = None
        prevBlock = None
        
        self.skipBlocks = 0
        self.ramBlocks = 0
        self.regionSwitches = []
        self.ordered = True
        
        self.minAddr = _CEofRAM
        self.maxAddr = 0
        self.dataStat = StatElement()
        self.zeroStat = StatElement()      
        
        for i, b in enumerate(blockList):
            
            if b.NonRam():
                if (prevBlock is not None) and (not prevBlock.NonRam()):
                    self.regionSwitches.append(i)
                self.skipBlocks += 1
                prevBlock = b
                continue
            
            if (prevBlock is not None) and (prevBlock.NonRam()):
#                print("Switch: " + str(prevBlock) + " -> " + str(b))
                self.regionSwitches.append(i)
                        
            self.ramBlocks += 1
            prevBlock = b
                        
            # order and not crossed
            if prevRamBlock is not None:
                if b.Addr < prevRamBlock.Addr + prevRamBlock.Size:
                    self.ordered = False
                    
            # stats
            if b.Addr < self.minAddr: self.minAddr = b.Addr            
            if b.EndAddr > self.maxAddr: self.maxAddr = b.EndAddr
            
            if b.ZeroFilled: self.zeroStat.Add(b.Size)
            else:            self.dataStat.Add(b.Size)
               
            # next
            prevRamBlock = b
                        
    def __str__(self):
        
        fullRange = self.maxAddr - self.minAddr + 1
        free = fullRange - self.dataStat.Sum - self.zeroStat.Sum
        s = "LDR info: RAM blocks {0}, ignored {1}, ordered {2}, switches {3}".format(
            self.ramBlocks, self.skipBlocks, self.ordered, self.regionSwitches)
        s += "\n    min addr {0} size {1}".format(self.minAddr, IntToSize(fullRange))
        s += "\n    data {0} zero {1} free {2}".format(
            IntToSize(self.dataStat.Sum), IntToSize(self.zeroStat.Sum), IntToSize(free))
        s += "\n    zeroStats {0}".format(str(self.zeroStat))
        s += "\n    dataStats {0}".format(str(self.dataStat))
                
        return s
    
    @property
    def MaxAddr(self) -> int:
        return self.maxAddr
    
    def CorrectForSplit(self) -> bool:
        return (self.ordered == True) and (len(self.regionSwitches) == 2) 
    
    @property
    def RegionSwitches(self) -> [int]:
        return self.regionSwitches

# ----------------------------------------------------------

class LdrOutput:
    
    def __init__(self):
        self.data = bytearray()
        
    def AddBlock(self, addr : int, data : bytearray):
        
        def DataIsZero(data : bytearray) -> bool:
            for b in data: 
                if b != 0: return False
            return True
        
        count = len(data)        
        zero = DataIsZero(data)        
        flag = SetBit(0, _CCpuFlagBit)
        if zero: flag = SetBit(flag, _CZeroFlagBit)
         
        self.data += struct.pack('IIH', addr, count, flag) 
        if not zero: self.data += data 
    
    def AddRawBlock(self, b : Block):
        self.data += b.FullBinaryBody()
    
    def Save(self, fileName):
        with io.open(fileName, mode = "w+b") as f:
            f.write(self.data)
                              
# ----------------------------------------------------------
                
CDefaultBlockSize = 8 * 1024
                
class LdrDump:
        
    def __init__(self, fileName : str):
        with io.open(fileName, mode = "r+b") as f:
            data = f.read()
        
        self.fileName = fileName
        self.info = None
        self.blocks = []
        offs = 0
        
        while True:
            block = Block(data, offs)        
            self.blocks.append(block)
            if block.Final: break
            offs = block.NextBlockOffset()
            
    @property
    def FileName(self): return self.fileName
            
    def __PrintDebug(self):        
        lastBlock = self.blocks[-1:][0]
        s = ListClassFields(lastBlock, ignores = ["m_data"])
        print(s)
        
        indx = list(range(10)) + list(range(len(self.blocks) - 10, len(self.blocks)))
        for i in indx:
            print(self.blocks[i])
            
    def DumpFrom(self, addr, count : int = 16) -> str:
        block = None
        for b in self.blocks: 
            if b.AddrInside(addr):
                block = b
                break 
        
        if block is None: return ""
        
        dump = block.Dump(addr, count)
        dumpS = "{0}: {1}".format(IntToHex(addr), DumpBytes(dump)) 
        dumpS += "; file offs: " + IntToHex(block.AddrToOffsetInFile(addr))
        return dumpS + "; from block (" + str(block) + ")"
        
    @property
    def Info(self):
        if self.info is None: self.info = BlocksStats(self.blocks)
        return self.info
    
    def TopTen(self) -> str:
        blocks = sorted(self.blocks, key = lambda b: b.Size, reverse = True)
        
        res = []
        for i, b in enumerate(blocks):
            if i >= 10: break
            res.append(str(b))
            
        return "\n".join(res)
    
    def GetRamDump(self, blockSize : int = CDefaultBlockSize) -> [bytearray]:
        
        assert self.Info.CorrectForSplit()
        switches = self.Info.RegionSwitches
        
        # make dump
        ramDumpSize = self.Info.MaxAddr + 1
        divLast = ramDumpSize % blockSize 
        if divLast != 0: ramDumpSize += (blockSize - divLast)
        assert ramDumpSize % blockSize == 0
          
        ramDump = bytearray(b'\x00' * ramDumpSize)
                
        for indx in range(switches[0], switches[1]):
            b = self.blocks[indx]
            assert not b.NonRam()
            data = bytearray(b.Data)
            ramDump[b.Addr:b.Addr + len(data)] = data
            
        assert len(ramDump) == ramDumpSize
        
        return ramDump
                                
    def Split(self, resultFileName : str, blockSize : int = CDefaultBlockSize, verify = True):
        
        switches = self.Info.RegionSwitches
        
        # make dump
        ramDump = self.GetRamDump(blockSize)
        
        # output LDR
        ldr = LdrOutput()
        
        def AddRaw(xrange):
            for indx in xrange:
                b = self.blocks[indx]
                assert b.NonRam() 
                ldr.AddRawBlock(b)
                    
        # blocks before RAM
        AddRaw( range(0, switches[0]) )
            
        # RAM
        count = len(ramDump) // blockSize
        for i in range(count):
            addr = i * blockSize
            ldr.AddBlock(addr, ramDump[addr:addr + blockSize])
                
        # after RAM
        AddRaw( range(switches[1], len(self.blocks)) )
        
        # save
        ldr.Save(resultFileName)
        
        # compare
        if verify:
            resultDump = LdrDump(resultFileName).GetRamDump(blockSize = blockSize)
            assert ramDump == resultDump

# ----------------------------------------------------------

def _RunForOptions(args : [str]):
    
    if len(args) == 0:
        CHelp = r"Usage: LdrFileIn [-info] [-dump AddrHex Size] [-repack BlockSizeKb LdrFileOut>]" 
        print(CHelp)
        exit()
        
    argKeys = SplitCommandLineByKeys(args)
    
    ldrFile = argKeys[0][0]
    if not os.path.exists(ldrFile): 
        print("File {0} not exists!".format(ldrFile))
        exit()
    ldr = CreateLazyInitProxy(lambda: LdrDump(ldrFile))
    
    def _LdrRepack(blockSize : int, fileName : str):
    
        cacheFileName = fileName + ".cache"
        
        if os.path.exists(fileName):        
            if QuickFileCache.VerifyCache(cacheFileName):
                print('LDR file cached!')
                return
        
        print("Repacking to {0} block size {1}...".format(fileName, blockSize))
        ldr.Split(fileName, blockSize)
        QuickFileCache.Save(ldr.FileName, cacheFileName) # make cache 
        print("Done")
        
    def GetSize(s : str) -> int:
        if s[-1:] == "b": return int(s[:-1])
        return int(s) * 1024
         
    for opt in argKeys[1:]:
        optName = opt[0]
        
        if optName == "-info":
            assert len(opt) == 1
            print( ldr.Info )
            
        elif optName == "-dump":
            assert len(opt) == 3
            addr = int(opt[1], 16)
            size = int(opt[2])
            print(ldr.DumpFrom(addr, size))
            
        elif optName == "-repack":
            assert len(opt) == 3
            blockSize = GetSize(opt[1])
            fileName = opt[2]
            _LdrRepack(blockSize, fileName)
            
        else:
            raise Exception('Bad option name {0}!'.format(optName))
        

# ----------------------------------------------------------

if __name__ == '__main__':
    
    _RunForOptions(sys.argv[1:])
             
# ----------------------------------------------------------

#if __name__ == '__main__':
#    
##    CFileName = r"d:\share\BooterBin\BfBoot.ldr"
##    CFileName = r"d:\proj\Ipsius\Blackfin\BfLdrCheck\Debug\BfLdrCheck.ldr"
##    CFileName = r"d:\proj\Ipsius\Blackfin\BfMainE1\Debug\BfMainE1.ldr"
#    CFileName = r"d:\proj\IpsiusBfBadLdr\Blackfin\BfBoot\Debug\BfBoot.ldr"
#    
#    dump = LdrDump(CFileName)
#
#    CDumpAddr = 0x16b0
#    print( dump.DumpFrom(CDumpAddr) )
#    
#    print( dump.Info )
#    print( dump.TopTen() )
#    
#    dump.Split(r"c:\a.ldr")
#    print("OK")
    