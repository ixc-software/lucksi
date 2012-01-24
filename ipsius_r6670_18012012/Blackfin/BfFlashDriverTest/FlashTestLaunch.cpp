#include "stdafx.h"

#include "FlashTestLaunch.h"


#include <VDK.h>

#include "Utils/ErrorsSubsystem.h"
#include "Platform/PlatformTypes.h"

#include "BfDev/BfFlashDriver.h"
#include "BfDev/BfFlashDriverTest.h"

using namespace Platform;
using namespace BfDev;
using namespace std;


// -----------------------------------------------------------------------------


void TestTimer(Platform::dword oneLedTimeout)
{
    word leds[6]    = {PF6, PF7, PF8, PF9, PF10, PF11};
    word buttons[4] = {PF2, PF3, PF4, PF5};

	*pPORTFIO_DIR  |= (leds[0] | leds[1] | leds[2] | leds[3] | leds[4] | leds[5]);
	*pPORTFIO_INEN  = (buttons[0] | buttons[1] | buttons[2] | buttons[3]);
	*pPORTFIO_CLEAR = (leds[0] | leds[1] | leds[2] | leds[3] | leds[4] | leds[5]);

	dword led = 0;
    dword startTime = GetSystemTickCount();
	while (true)
    {

        while (GetSystemTickCount() - startTime <= oneLedTimeout)
        {
            if ( (*pPORTFIO & (buttons[0] | buttons[1] | buttons[2] | buttons[3])) )
                return;
        }

        *pPORTFIO ^= leds[led];
        led = (led != 5) ? (led + 1) : 0;

        startTime = GetSystemTickCount();
    }
}


// -----------------------------------------------------------------------------


void TestTimer()
{
    TestTimer(10000); // Toggle one by one each led every 6 seconds
}


// --------------------------------------------

void FlashTestLaunch()
{
    // Dump sector map
    {
        BfFlashDriver flashDriver(BfFlashProfile::M29W320());

        string sectorMap;
        flashDriver.printSectorsInfoTo(sectorMap);

        cout << sectorMap << endl;
    }


//    TestTimer();


    BfFlashDriverTest::TestFlashDriver(BfFlashProfile::M29W320());

    string benchmarkLog;
    BfFlashDriverTest::BenchmarkAll(benchmarkLog, BfFlashProfile::M29W320());
    cout << benchmarkLog.c_str() << endl;    
}
