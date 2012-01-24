
#ifndef __CPUUSAGETHREADTEST__
#define __CPUUSAGETHREADTEST__

// CPUUsageThreadTest.h
#include "Platform/PlatformTypes.h"

namespace UtilsTests
{
    void CPUUsageThreadTest(bool silentMode = true);
    void RunMonitoring(Platform::dword CPUCheckCount, Platform::dword sleepBetweenChecksMs);

    void Win32CpuUsageTest();
    
} // namespace UtilsTests

#endif
