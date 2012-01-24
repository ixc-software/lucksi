
#ifndef __FlashDumpImpl__
#define __FlashDumpImpl__

// FlashDumpImpl.h

#include "NObjMiniFlasher.h"
#include "MfProcess.h"

namespace MiniFlasher
{
    using Platform::byte;
    
    class FlashDumpImpl : IMfProcessUser
    {
    // IMfProcessUser impl;
    private:
        void RunImpl(MfProcess &p) // can throw
        {
            if (p.Params().FileName.isEmpty()) MfProcess::ThrowErr("File name is empty.");
        
            dword size = MfProcUtils::CorrectSize(p.Params().FlashSize, p.Params().FlashOffset);
            
            p.AddLogHeader("Flash dump obtaining", p.Params().FileName, size); 
    
            // read & save
            p.ProtocolConnect();
            p.ProtocolFlashDump(size, p.Params().FlashOffset, p.Params().FileName);
        }
        
    public:
        // Required parameters:
        // - uart profile
        // - file name
        // - flash offset
        // - size of flash to copy
        FlashDumpImpl(MfProcessParams params)
        {
            MfProcess p(params, true);
            p.Run(*this);
        }
    };

    // --------------------------------------------------------

    class FlashDumpVerifyImpl : IMfProcessUser
    {
    // IMfProcessUser impl;
    private:
        void RunImpl(MfProcess &p) // can throw
        {
            QFileInfo f(p.Params().FileName);
        
            p.AddLogHeader("Flash dump verification", f.absoluteFilePath(), f.size());
    
            // read file
            QByteArray toVerify;
            MfProcUtils::ReadFile(f, toVerify);
    
            // read flash
            p.ProtocolConnect();
            QByteArray dump = p.ProtocolFlashDump(toVerify.size(), p.Params().FlashOffset);
            
            // check
            if (dump == toVerify) p.AddLog("Verified: OK.");
            else p.AddLog("Verified: Fail.");
        }
        
    public:
        // Required parameters:
        // - uart profile
        // - file name
        // - flash offset
        FlashDumpVerifyImpl(MfProcessParams params)
        {
            MfProcess p(params, true);
            p.Run(*this);
        }
    };
    
} // namespace MiniFlasher

#endif
