
#include "stdafx.h"

#include "iCore/MsgThread.h"
#include "Platform/Platform.h"

#include "ThreadCounterTest.h"

// --------------------------------

using namespace Platform;

namespace
{
    int GInsideThreadCount;

    class TestThread : public Platform::Thread
    {

        void run()  // override
        {
            GInsideThreadCount = GetCurrentProcessThreadCount();
        }

    public:

        TestThread() : Platform::Thread("TestThread")
        {
            start(HighPriority);
        } 
        
    };
}

// ---------------------------------

namespace PlatformTests
{
    
    void GetCurrentProcessThreadCountTest(bool printInfo)
    {    
        dword before = GetCurrentProcessThreadCount();
        
        {
            TestThread thread;
            thread.wait();
        }
    
        dword after = GetCurrentProcessThreadCount();
    
        if (printInfo)
        {
            using namespace std;

            cout << std::endl 
                    << "Threads in process : " << before << endl
                    << "Create & destroy 2 threads" << endl
                    << "Threads in process: " << after << endl;
        }
    
        TUT_ASSERT(before == after);
        TUT_ASSERT(GInsideThreadCount == (before + 1));
    }

} // namespace PlatformTests
