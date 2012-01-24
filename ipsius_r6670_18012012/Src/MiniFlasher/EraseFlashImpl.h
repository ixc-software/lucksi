
#ifndef __ERASEFLASHIMPL__
#define __ERASEFLASHIMPL__

// EraseFlashImpl.h

#include "NObjMiniFlasher.h"
#include "MfProcess.h"


namespace MiniFlasher
{
    class EraseFlashImpl : IMfProcessUser
    {
    // IMfProcessUser impl;
    private:
        void RunImpl(MfProcess &p) // can throw
        {
            dword offset = 0;
            dword size = MfProcUtils::CorrectSize(p.Params().FlashSize, offset);

            p.AddLogHeader("Erase flash", "", size);
            p.ProtocolConnect();
            p.ProtocolFlashErase(size, offset);
        }
        
    public:
        // Required parameters:
        // - uart profile
        // - flash offset = 0
        // - flash size
        EraseFlashImpl(MfProcessParams params) 
        {
            MfProcess p(params, true);
            p.Run(*this);
        }
    };
    
} // namespace MiniFlasher


#endif
