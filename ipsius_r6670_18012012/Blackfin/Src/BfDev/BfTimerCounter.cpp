#include "stdafx.h"
#include "Platform/Platform.h"
#include "BfTimerCounter.h"

// -------------------------------------

namespace
{
    int GUsedTimersMask = 0;
}

namespace BfDev
{

    // warning! this is not thread safe code!
    void BfTimerCounter::AllocTimer(byte timerNumber)
    {
        int mask = (1 << timerNumber);

        if (GUsedTimersMask & mask) ESS_HALT("BfTimerCounter reuse!");

        GUsedTimersMask |= mask;
    }

    void BfTimerCounter::FreeTimer(byte timerNumber)
    {
        int mask = (1 << timerNumber);

        if ((GUsedTimersMask & mask) == 0) ESS_HALT("BfTimerCounter free unused!");

        GUsedTimersMask ^= (mask);
    }

    bool BfTimerCounter::TimerEnabled(byte timerNumber)
    {
        dword mask = (1 << timerNumber);
        return ((*pTIMER_ENABLE) & mask) ? true : false;
    }


    int BfTimerCounter::GetFreeTimer()
    {
        for(int i = 0; i < CMaxTimers; ++i)
        {
            bool usedByMask = ((GUsedTimersMask & (1 << i)) != 0);
            bool enabled = TimerEnabled(i);
            if ((!usedByMask) && (!enabled)) return i;
        }

        return -1;
    }

    // -------------------------------------------------

    void BfTimerTestLoop()
    {
        using namespace std;

        BfTimerCounter tmr(1);

        int lastTime = Platform::GetSystemTickCount();
        int lastTicks = tmr.GetCounter();

        for(int i = 0; i < 4; i++)
        {
            Platform::ThreadSleep(5);

            int currTime = Platform::GetSystemTickCount();
            int currTicks = tmr.GetCounter();

            cout << "Time " << (currTime - lastTime) << endl;
            cout << "Ticks " << tmr.CounterToMcs(currTicks - lastTicks) << " mcs" << endl;

            lastTime = currTime;
            lastTicks = currTicks;
        }
    }
    
    void BfTimerReset(int timerIndx)
    {        
        const int CCount = 1024;

        BfTimerCounter t(timerIndx);
        
    	while(t.GetCounter() < CCount);
    	
    	t.Reset();
    	
    	int val = t.GetCounter();
    	
    	ESS_ASSERT(val < CCount);    	
    }
	
    void BfTimerCounterTest()
    {
    	BfTimerReset(1);
        BfTimerReset(7);
    	
        BfTimerTestLoop();
        BfTimerTestLoop();
    }
    
    void BfTimerReserveTimersMask(dword mask)
    {
    	ESS_ASSERT((GUsedTimersMask & mask) == 0);
    	
        GUsedTimersMask = mask;
    }
	
}  // namespace BfDev
