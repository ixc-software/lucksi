
#include "stdafx.h"

#include "ThreadWaitUSTest.h"
#include "Platform/Platform.h"
#include "iCore/SyncTimer.h"
#include "iCore/MsgThread.h"
//#include "Utils/ErrorsSubsystem.h"
#include "Utils/AtomicTypes.h"

using namespace Platform;

namespace
{
    const int CTestTimeUS = 50; // us
    const int CIntervalMS = 1; //ms = 1000 * us

    enum { CTimeOutMs = 1000 };

    using namespace Utils;
    using namespace iCore;
}

// --------------------------------

namespace PlatformTests
{
    void ThreadWaitUSTest()
    {
        int count = CIntervalMS *1000 / CTestTimeUS;
        SyncTimer sTimer;
    
        //sTimer.Get() - time since SyncTimer obj created
        for (int i = 0; i < count; ++i)
        {
            ThreadWaitUS(CTestTimeUS);
        }
    
        dword interval = sTimer.Get() + 1;
    
        //1 - miscalculation 
        TUT_ASSERT(interval >= (CIntervalMS));
    
        //std::cout << "interval = " << interval;
    } 

    // ----------------------------------------
    
    void ThreadMinimalSleepTest()
    {
        class TestThread : public Platform::Thread
        {
            AtomicInt m_val;
    
            void run()
            {
                m_val.Inc();
            }
    
        public:
            TestThread() : Platform::Thread("TestThread")
            {
                start(LowPriority);
            }
        
            int getVal()
            {
                return m_val;
            }
        };
    
        TestThread testThread;
    
        iCore::SyncTimer t;
    
        while(t.Get() < CTimeOutMs)
        {
            ThreadMinimalSleep();
            if (testThread.getVal() > 0) 
            {
            	testThread.wait();
            	return;  // test ok
            }
            
        }
    
        TUT_ASSERT(0 && "Timeout!");
    }

} // namespace PlatformTests
