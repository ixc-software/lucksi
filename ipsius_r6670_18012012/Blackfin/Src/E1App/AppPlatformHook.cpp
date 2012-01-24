#include "stdafx.h"

#include "Utils/ErrorsSubsystem.h"
#include "BfDev/BfUartSimpleCore.h" 
#include "BfDev/SysProperties.h"
#include "BfDev/BfTimerCounter.h"
#include "BfDev/BfWatchdog.h"
#include "DevIpTdm/BfLed.h"
#include "iVDK/CriticalRegion.h"

#include "AppPlatformHook.h"

// ------------------------------------------------------

namespace
{
    using Platform::byte;
    using BfDev::BfUartSimpleCore;

    bool GUseUart;
    bool GAutoReboot;
    int  GSysFreq;
    int  GUartSpeed;

    byte GUartBody[sizeof(BfUartSimpleCore)];

    void ExitHandle(const char *pMsg)
    {
        const int CPeriodMs = 500;

        // loop forever
        iVDK::CriticalRegion::Enter();

        BfUartSimpleCore *pUart = 0;
        if (GUseUart) pUart = new(GUartBody) BfUartSimpleCore(GSysFreq, 0, GUartSpeed);

        BfDev::BfTimerCounter t(0, GSysFreq, true);
        int interval = t.NsToCounter(CPeriodMs * 1000 * 1000);

        int color = DevIpTdm::OFF;
        while (true)
        {    
            DevIpTdm::BfLed::SetColor(color);
            color = (color == DevIpTdm::OFF) ? DevIpTdm::RED : DevIpTdm::OFF;
            t.Sleep(interval);

            if (pUart != 0) 
            {
                pUart->Send(pMsg);
                pUart->Send("\n\r");
            }

            if (GAutoReboot)
            {
        		int interval = t.NsToCounter(500 * 1000 * 1000);            	
                t.Sleep(interval);

                // bye-bye
                BfDev::BfWatchdog::SwReset();
            }
        }

    }

}

// ------------------------------------------------------

namespace E1App
{

    void AppPlatformHookSetup(bool enabled, bool useUart, bool autoReboot, int uartSpeed)
    {
        if (enabled)
        {
            GUseUart = useUart;
            GAutoReboot = autoReboot;
            GSysFreq = BfDev::SysProperties::Instance().getFrequencySys();
            GUartSpeed = uartSpeed;
            Platform::SetExitHandle(ExitHandle);
        }
        else
        {
            Platform::SetExitHandle(0);
        }
    }
    
}  // namespace E1App
