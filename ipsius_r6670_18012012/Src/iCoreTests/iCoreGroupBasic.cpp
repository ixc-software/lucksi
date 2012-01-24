
#include "stdafx.h"

#include "iCoreGroupBasic.h"

#include "iCore/MsgThread.h"
#include "iCore/SyncTimer.h"
#include "Utils/ErrorsSubsystem.h"
#include "Utils/AtomicTypes.h"
#include "Utils/ExeName.h"

#include "iCoreSimpleMsgTest.h"
#include "iCoreMsgBenchmarks.h"
#include "iCoreSynchronSendingMsgTest.h"
#include "iCoreTimerTest.h"
#include "iCoreTimerTestAdvanced.h"
#include "iCoreThreadPoolTest.h"
#include "iCoreThreadPoolTestA.h"
#include "iCoreAsyncTests.h"
#include "iCoreExceptionHooks.h"
#include "iCoreOverload.h"


namespace
{
    const int CBenchTime = 1 * 1000;

	// ----------------------------------------------

	void CreateDestroyMsgThreadTest()
	{
		iCore::MsgThread m("CreateDestroyMsgThreadTest");
		Platform::Thread::Sleep(100);
	}

	// ----------------------------------------------

    void AppendToFile(const QString &file, const std::string &data)
    {
        ESS_ASSERT(!file.isEmpty());
        if (data.empty()) return;

        QFile f(file);
        if (!f.open(QFile::WriteOnly | QFile::Append)) return;
        f.write(data.c_str(), data.size());
        f.close();
    }
    
    // ----------------------------------------------
    
	// single thread benchmark
	void SingleThreadBenchmark(QString file)
	{
        AppendToFile(file, iCoreTests::iCoreBenchmarkSingleN(CBenchTime, 1));
	}

	// ----------------------------------------------

	// multi thread benchmark
	void MultiThreadBenchmark(QString file)
	{
		AppendToFile(file, iCoreTests::iCoreBenchmarkMulti(CBenchTime));
	}

	// ----------------------------------------------

	// single thread benchmark (two threads)
	void SingleThreadBenchmarkNThreads(QString file)
	{
		AppendToFile(file, iCoreTests::iCoreBenchmarkSingleN(CBenchTime, 2));
	}
   
	// ----------------------------------------------

    using namespace TestFw;
	using namespace iCoreTests;

    class iCoreTestsGroup : public TestGroup<iCoreTestsGroup>
    {
        iCoreTestsGroup(ILauncher &launcher) : TestGroup<iCoreTestsGroup>(launcher, "iCoreTests")             
        {
            QString logFile(Utils::ExeName().GetExeDir().c_str());
            logFile.append("MsgBenchmarks.txt");
            if (QFile::exists(logFile)) QFile::remove(logFile);
            
            bool printInfo = false;

            Add("Creation/destruction MsgThread test",		&CreateDestroyMsgThreadTest);
            Add("Sending msg through MsgThread test", 		&iCoreSimpleMsgTest);
            Add("Single thread benchmark", 					&SingleThreadBenchmark,         logFile);
			Add("Multi thread benchmark", 					&MultiThreadBenchmark,          logFile);
			Add("Single thread benchmark (two threads)",    &SingleThreadBenchmarkNThreads, logFile);
			Add("Unavailability of msg sending "
				"after calling ~MsgThread() test",			&iCoreSignatureCheck);
			Add("Receiving synchron sent msgs test", 		&iCoreSynchronSendingMsgTest);
			Add("Timer simple test", 						&iCoreTimerTest);
			Add("Timer's BindEvent() and tags test",		&iCoreTimerAdvancedTest);
			Add("Thread pool test (part 1)",				&iCoreThreadPoolTest);
			Add("Thread pool test (part 2)", 				&iCoreThreadPoolTestA, printInfo);
			// obsolete
			// Add("MsgObjectLink test", 					&iCore::MsgObjectLinkTest);
			Add("MsgThread::RunAsyncTask() test", 			&iCoreAsyncTests);
			Add("iCore::ThreadRunner events test", 			&iCoreExceptionHooks);
			Add("Freeing T* in destructor MsgParam test",   &iCoreParamMsgTest);
			// ... 

            // overload detection
            Add("Watchdog for hang-up message",             &TestBlockedMsgHalt);

            Add("MsgThreadMonitor: max msg in queue",       &TestMaxMsgInThreadHalt);
            Add("MsgThreadMonitor: timer lag detection",    &TestMaxTimerLag);
            Add("MsgThreadMonitor: msg grow",               &TestMsgGrow);
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


