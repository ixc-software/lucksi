#include "stdafx.h"
#include "Utils/ErrorsSubsystem.h"
#include "TestBroadcastMsgConversion.h"
#include "BfBootCore/BroadcastMsg.h"

namespace 
{
    using Platform::dword;
    using Platform::dword;
    using Platform::word;
    using Platform::byte;
    
    class T
    {
        Utils::HostInf m_srcAddress;
    };

    void TestSimple()
    {
      

        const bool CIsBooter = true;

        const std::string CAddr = "0.0.0.0";
        Utils::HostInf CHost(CAddr, 1111);

        const std::string CMac = "00";
        const dword CHwNumber = 0;
        const dword CHwType = 0;
        const std::string CBoterVersion = "ver";
        const word CPort = 8979;
        const word CRelVer = 3;
        BfBootCore::ImgDescrList imgs;
        BfBootCore::ImgDescriptor descr;
        descr.Id = 0; descr.Name = "descrName"; descr.Size = 100;
        imgs.Add(descr);
        imgs.Add(descr);

        int cmp = 0;
        int cbp = 0;
        if(CIsBooter)   cbp = CPort;
        else            cmp = CPort;
        BfBootCore::BroadcastMsg msg; //(CBoterVersion, CHwNumber, CHwType, cbp, cmp, CMac, imgs, CRelVer);
        msg.BooterVersionInfo = CBoterVersion; 
        msg.HwNumber = CHwNumber;
        msg.HwType = CHwType;
        msg.BooterPort = cbp;
        msg.CmpPort = cmp;
        msg.Mac = CMac;
        msg.SoftReleaseInfo = imgs;
        msg.SoftRelNumber = CRelVer;

        //T t;
        std::vector<byte> bin;         
        msg.ToBin(bin);
        TUT_ASSERT(!bin.empty());

        //msg.getSrcAddr();        

        BfBootCore::BroadcastMsg restoredMsg; 
        restoredMsg.Assign( CHost, &bin.at(0), bin.size() );
        
        TUT_ASSERT(restoredMsg.IsBooter() == CIsBooter);
        TUT_ASSERT(restoredMsg.Mac == CMac);
        TUT_ASSERT(restoredMsg.HwNumber == CHwNumber);
        TUT_ASSERT(restoredMsg.HwType == CHwType);
        TUT_ASSERT(restoredMsg.BooterVersionInfo == CBoterVersion);
        TUT_ASSERT(restoredMsg.BooterPort == (CIsBooter ? CPort : 0) );
        TUT_ASSERT(restoredMsg.CmpPort == (CIsBooter ? 0 : CPort) );
        TUT_ASSERT(restoredMsg.SoftReleaseInfo == imgs);     
        TUT_ASSERT(restoredMsg.CbpAddress.Address() == CAddr);
        TUT_ASSERT(restoredMsg.CbpAddress.Port() == CPort);

        
    }
    
} // namespace 

namespace TestBfBoot
{
    void TestBroadcastMsgConversion()
    {               
        TestSimple();

        // todo
        // TestThrow();
    }
} // namespace TestBfBoot
