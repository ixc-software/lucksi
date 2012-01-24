#include "stdafx.h"

#include <defBF537.h>

#include "Platform/PlatformTypes.h"
#include "Utils/ErrorsSubsystem.h"
#include "BfDev/SysProperties.h"

#include "BfWatchdog.h"

using Platform::dword;
using Platform::ddword;

// ----------------------------------------------------

namespace
{

    void WatchdogStart(dword counter)
    {
        *pSYSCR = *pSYSCR & (~NOBOOT); // reset NOBOOT bit            

        *pWDOG_CNT = counter;
        *pWDOG_CTL = WDEN | WDEV_RESET;  
    }

    void WatchdogStop()
    {
        *pWDOG_CTL = WDDIS;
    }

    void WatchdogReset()
    {
        *pWDOG_STAT = 0; // dummy write for service
    }

    static BfDev::BfWatchdog *GInst = 0;
}

// ----------------------------------------------------

namespace BfDev
{
        
    BfWatchdog::BfWatchdog( int timeoutMs )
    {
        ESS_ASSERT(GInst == 0);        
        GInst = this;

        dword sysFreq = BfDev::SysProperties::Instance().getFrequencySys();
        ddword counter = ((ddword)sysFreq * timeoutMs) / 1000;

        ESS_ASSERT(counter < std::numeric_limits<dword>::max());

        WatchdogStart(counter);
    }

    void BfWatchdog::Reset()
    {
        WatchdogReset();
    }

    BfWatchdog::~BfWatchdog()
    {
        GInst = 0;

        WatchdogStop();
    }

    void BfWatchdog::SwReset()
    {
        if (GInst != 0) WatchdogStop();

        WatchdogStart(1);
        while(true);
    }

}  // namespace BfDev

