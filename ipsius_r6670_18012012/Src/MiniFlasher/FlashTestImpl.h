
#ifndef __FLASHTESTIMPL__
#define __FLASHTESTIMPL__

// FlashTestImpl.h

#include "NObjMiniFlasher.h"
#include "MfProcess.h"
#include "MfPacketFieldNames.h"
#include "MfUtils.h"

namespace MiniFlasher
{
    class FlashTestImpl : IMfProcessUser
    {
    // IMfProcessUser impl;
    private:
        void RunImpl(MfProcess &p) // can throw
        {
            using namespace MfPacketFieldNames;

            p.AddLogHeader("Flash test", "", MfProcUtils::MaxFlashSize());
            
            p.ProtocolConnect();
    
            p.AddLog("Running ... ");
            p.Protocol().StartFlashTest();
            p.ProgressBar().Start(100); // %
    
            Utils::TimerTicks timer;
            dword timeout = p.Params().Timeouts.CommandTimeout;
            dword errors = 0;
            while(true)
            {
                const MfPacket *pPack = p.ProtocolWaitPacket(timeout); // can throw
                
                if (pPack->Type() == Protocol::RespFlashTestProgress)
                {
                    p.ProgressBar().PrintStep(pPack->Get(FPercent).AsDword());
                    continue;
                }
                
                if (pPack->Type() == Protocol::RespFlashTestCompleted)
                {
                    p.ProgressBar().Finish(true);
                    errors = pPack->Get(FErrors).AsDword();
                    break;
                }
    
                MfProcess::ThrowUnexpectedPacketErr(*pPack);
            }
            
            p.AddLog(MiniFlasherUtils::OperationInfo("Running", timer.Get()), false);
            p.AddLog(QString(" Errors: %1").arg(errors));
        }
        
    public:
        // Required parameters:
        // - uart profile
        FlashTestImpl(MfProcessParams params)
        {
            MfProcess p(params, true);
            p.Run(*this);
        }
    };
    
} // namespace MiniFlasher

#endif
