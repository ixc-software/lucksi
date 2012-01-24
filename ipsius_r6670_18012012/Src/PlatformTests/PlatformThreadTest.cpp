#include "stdafx.h"
#include "Platform/Platform.h"
#include "PlatformTests.h"
#include "Utils/TimerTicks.h"
#include "Utils/ErrorsSubsystem.h"

// -------------------------------------

namespace
{

    enum
    {
        CSleepInterval = 100,
        CTimeout = CSleepInterval * 8,
    };

    enum ThreadState
    {
        TsInit,
        TsMainThreadReady,
        TsChildThreadReady,
    };

    class Thread : public Platform::Thread
    {
        ThreadState &m_state;

        void run()  // override
        {
            while(true)
            {
                Sleep(CSleepInterval);

                if (m_state == TsMainThreadReady)
                {
                    m_state = TsChildThreadReady;
                    break;
                }
            }
        }

    public:

        Thread(ThreadState &state) : Platform::Thread("Thread"), m_state(state)
        {
            start();
        }

    };

    void Test()
    {
        // Platform::Thread::CurrentThreadPriority(Platform::Thread::NormalPriority);

        ThreadState state = TsInit;

        Thread thread(state);

        Utils::TimerTicks t;
        t.Set(CTimeout);

        while(true)
        {
            Thread::Sleep(CSleepInterval);

            if (t.TimeOut()) TUT_ASSERT(0 && "Timeout");

            if (state == TsInit) state = TsMainThreadReady;
            if (state == TsChildThreadReady) break;  // ok
        }

        thread.wait();

    }
}



// -------------------------------------

namespace PlatformTests
{
	
	void ThreadTest()
	{
		Test();
    }
		
}  // namespace PlatformTests


