
#include "stdafx.h"

#include "iCoreGroupBasic.h"

#include "iCore/MsgThread.h"
#include "iCore/SyncTimer.h"
// #include "Utils/TestLog.h"
#include "Utils/ErrorsSubsystem.h"
#include "Utils/AtomicTypes.h"

#include "iCoreSimpleMsgTest.h"
#include "iCoreMsgBenchmarks.h"
#include "iCoreTimerTest.h"

/*
#include "iCoreSynchronSendingMsgTest.h"
#include "iCoreTimerTestAdvanced.h"
#include "iCoreThreadPoolTest.h"
#include "iCoreThreadPoolTestA.h"
#include "iCoreAsyncTests.h"
#include "iCoreExceptionHooks.h"
#include "iCoreOverload.h"
*/


namespace
{

	void CreateDestroyMsgThreadTest()
	{
		iCore::MsgThread thread;
		thread.Run();
		Platform::ThreadSleep(10);
		thread.Break();			
	}

	// ----------------------------------------------
    
	// single thread benchmark
	void SingleThreadBenchmark(bool printInfo)
	{
		const int CBenchTime = 100;
		
		std::string res = iCoreTests::iCoreBenchmarkSingle(CBenchTime);
		
		if (printInfo) std::cout << res << std::endl;
	}

	// ----------------------------------------------
	
    using namespace TestFw;
	using namespace iCoreTests;

    class iCoreTestsGroup : public TestGroup<iCoreTestsGroup>
    {
        iCoreTestsGroup(ILauncher &launcher) : TestGroup<iCoreTestsGroup>(launcher, "iCoreBasic")             
        {
			bool printInfo = false;
			
            Add("Creation/destruction MsgThread test",		&CreateDestroyMsgThreadTest);
            Add("Sending msg through MsgThread test", 		&iCoreSimpleMsgTest);
            Add("Single thread benchmark", 					&SingleThreadBenchmark, printInfo);
			
            /*
			Add("Multi thread benchmark", 					&MultiThreadBenchmark);
			Add("Single thread benchmark (two threads)",    &SingleThreadBenchmarkNThreads);
			Add("Unavailability of msg sending "
				"after calling ~MsgThread() test",			&iCoreSignatureCheck);
			Add("Receiving synchron sent msgs test", 		&iCoreSynchronSendingMsgTest);
			Add("Timer simple test", 						&iCoreTimerTest);
			Add("Timer's BindEvent() and tags test",		&iCoreTimerAdvancedTest);
			Add("Thread pool test (part 1)",				&iCoreThreadPoolTest);
			Add("Thread pool test (part 2)", 				&iCoreThreadPoolTestA, printInfo);
			Add("MsgThread::RunAsyncTask() test", 			&iCoreAsyncTests);
			Add("iCore::ThreadRunner events test", 			&iCoreExceptionHooks);
			Add("Freeing T* in destructor MsgParam test",   &iCoreParamMsgTest);
			// ... 

            // overload detection
            Add("Watchdog for hang-up message",             &TestBlockedMsgHalt);

            Add("MsgThreadMonitor: max msg in queue",       &TestMaxMsgInThreadHalt);
            Add("MsgThreadMonitor: timer lag detection",    &TestMaxTimerLag);
            Add("MsgThreadMonitor: msg grow",               &TestMsgGrow);
            */
        }

    public:

        static void AddToLauncher(ILauncher &launcher)
        {
            new iCoreTestsGroup(launcher);  // free at launcher
        }
    };

} // namespace

// ------------------------------------------------

namespace iCoreTests
{
    void RegisterTestGroup(TestFw::ILauncher &launcher)
    {
        iCoreTestsGroup::AddToLauncher(launcher);
    }


} // namespace iCoreTests


