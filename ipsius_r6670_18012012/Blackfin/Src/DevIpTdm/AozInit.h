#ifndef __AOZINIT__
#define __AOZINIT__

#include "Utils/ErrorsSubsystem.h"

#include "BfDev/BfTimerCounter.h"

namespace DevIpTdm
{

    class AozInit
    {

        static bool& GetInitDone()
        {
            static bool initDone = false;
            return initDone;
        }
        
        static bool& GetTimersLocked()
        {
            static bool timersLocked = false;
            return timersLocked;
        }
                
        static const Platform::word CTimerMask = 0x0038;  // 111000b
        
    public:

        static bool InitDone()
        {
            return GetInitDone();
        }

        static bool InitSync()
        {
            ESS_ASSERT( GetTimersLocked() );
            ESS_ASSERT(!InitDone());

            Platform::word enabledMask = *pTIMER_ENABLE;
            if ((enabledMask & CTimerMask) != 0) return false;

            *pTIMER_DISABLE             |= CTimerMask;  
            *pPORTFIO_INEN              |= 0x8000;
            *pPORTF_FER                 |= 0x8070;

            *pTIMER3_CONFIG             = 0x00A9;   // !4096 KHz for TimeShift of TDM_CLK
            *pTIMER3_PERIOD             = 4;
            *pTIMER3_WIDTH              = 2;

            *pTIMER4_CONFIG             = 0x00AD;   // 2048 KHz for TimeShift of TDM_CLK
            *pTIMER4_PERIOD             = 8;
            *pTIMER4_WIDTH              = 4;

            *pTIMER5_CONFIG             = 0x00AD;  //AOZ_Frame
            *pTIMER5_PERIOD             = 2048;
            *pTIMER5_WIDTH              = 4;

            *pTIMER_ENABLE              |= CTimerMask;

            GetInitDone() = true;

            return true;
        }        
        
        static void LockTimers()
        {
            ESS_ASSERT( !GetTimersLocked() );
            
            BfDev::BfTimerReserveTimersMask(CTimerMask);
                
            GetTimersLocked() = true;
        }
    };
    
    
}  // namespace DevIpTdm


#endif
