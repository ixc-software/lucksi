
#ifndef __MFPROCESSPARAMS__
#define __MFPROCESSPARAMS__

#include "Platform/PlatformTypes.h"
#include "IMfProcessEvents.h"
#include "iUart/UartProfile.h"
#include "Utils/SafeRef.h"

namespace MiniFlasher
{
    using Platform::dword;

    struct MfProcessTimeouts
    {
        int CommandTimeout;
        int ConnectTimeout;
        int ConnectRepeatInterval;

        MfProcessTimeouts()
        {
            CommandTimeout = 20000;
            ConnectTimeout = 1500;
            ConnectRepeatInterval = 300;
        }
    };

    // ---------------------------------------------------------------

    struct MfProcessParams
    {
        Utils::SafeRef<IMfProcessEvents> Owner;
        iUart::UartProfile UartProfile;
        MfProcessTimeouts Timeouts;

        QString FileName;
        
        dword FlashOffset;
        dword FlashSize;
        bool RunFlashApp;

        MfProcessParams(const Utils::SafeRef<IMfProcessEvents> &owner,
                        const MfProcessTimeouts &timeouts):
            Owner(owner), UartProfile(-1), Timeouts(timeouts), FlashOffset(0), 
            FlashSize(0), RunFlashApp(false)
        {
            ESS_ASSERT(!Owner.IsEmpty());
        }

        static iUart::UartProfile DefaultUartProfile(int comPort, int baudRate, 
                                                     iUart::FlowType flowType = iUart::FlowOff)
        {
            return iUart::UartProfile(comPort, baudRate, iUart::DataBits_8, 
                                      iUart::StopBits_1, iUart::ParityNone, flowType);
        }
    };

    // ---------------------------------------------------------------

    
} // namespace MiniFlasher

#endif
