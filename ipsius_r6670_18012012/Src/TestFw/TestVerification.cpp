
#include "stdafx.h"

#include "TestVerification.h"

#include "iCore/SyncTimer.h"
#include "Platform/Platform.h"
#include "TestFw/TestFw.h"

// for self-test
#include "iCore/MsgThread.h" 
#include "Utils/AtomicTypes.h"
#include "Utils/ThreadNames.h"

namespace 
{
    const QThread::Priority CThreadPriority = QThread::NormalPriority;
    const int CTimeout = 60000; // ms
}

// -------------------------------------------

namespace TestFw
{
    using namespace Platform;

    TestVerification::TestVerification(dword timeout)
    : m_ignoreTimeout(false), m_pWasTimeout(0), m_timeout(timeout), 
		m_normalExit(true)
    {
    }

    // -------------------------------------------

    TestVerification::TestVerification(bool ignoreTimeout, Utils::AtomicBool *pWasTimeout,
									   dword timeout)
    : m_ignoreTimeout(ignoreTimeout), m_pWasTimeout(pWasTimeout), m_timeout(timeout),
		m_normalExit(true)
    {
    }

    // -------------------------------------------

	TestVerification::~TestVerification()
	{
        if (m_normalExit) return;

        // m_mutex.unlock();  // ?
        QThread::wait();
	}

	// -------------------------------------------

	void TestVerification::TestStarted(ISingleTest &test)
	{
        ESS_ASSERT(m_normalExit);
        
        // become true if only checks in TestCompleted() would be done 
        m_normalExit = false; 
        
        // check threads number
        m_threadCount = Platform::GetCurrentProcessThreadCount();

        m_mutex.lock();
        start(CThreadPriority);
	}

	// -------------------------------------------

	void TestVerification::TestCompleted(ISingleTest &test, const TestResult &result)
	{
        m_mutex.unlock();

        // wait for exit from run()
        QThread::wait();

        FinalCheck();
        m_normalExit = true;
	}

    // -------------------------------------------

    void TestVerification::run()
    {
        Utils::ThreadNamesLocker locker(QThread::currentThreadId(), "TestVerification");

        if (m_timeout == 0) return;

        // wait (true == test ok) 
        bool testOk = m_mutex.tryLock(m_timeout);

        if (!testOk) AssertTimeout();
        else m_mutex.unlock(); // only if we lock mutex in this thread
        
    }

    // -------------------------------------------

    void TestVerification::FinalCheck()
    {
        enum { CWaitToThreadTimeMs = 1000, };

        dword t = GetSystemTickCount();

        int after = 0;
        while((GetSystemTickCount() - t) < CWaitToThreadTimeMs)
        {
            after = GetCurrentProcessThreadCount();
            if (m_threadCount == after) return;
        }

        m_normalExit = false;

        // ESS_THROW(ThreadCountException);

        std::stringstream msg;
        msg << "Invalid number of threads: before = " << m_threadCount << ", after = " << after << '.';
        ESS_THROW_MSG( TestLauncher::VerificationException, msg.str() );
     }

    // -------------------------------------------

    void TestVerification::AssertTimeout()
    {
        m_normalExit = false;
        if (m_pWasTimeout != 0) m_pWasTimeout->Set(true);
        
        if (!m_ignoreTimeout) 
            ESS_ASSERT(0 && "Test timeout!");
        
    }

    // -------------------------------------------

    dword TestVerification::DefaultTimeout() 
    {
        return CTimeout; 
    }

	// -------------------------------------------

	void TestVerification::ThreadCountTest()
	{
        TestVerification testObj;

        // dummi
        SingleTestDummi t;
        ISingleTest *test = &t; 
        TestResult res;         

        {            			
			iCore::MsgThread thread("ThreadCountTest");  // +1 thread
			thread.Run();
            
			testObj.TestStarted(*test);                  // 2 threads	
        }                                                // -1 thread

        bool threadsCountTestOk = false;
		try
		{
			testObj.TestCompleted(*test, res);           // 1 thread (before != after)
		} 
		catch(/*const*/ TestLauncher::VerificationException &e)
		{
			threadsCountTestOk = true;
		}

		TUT_ASSERT(threadsCountTestOk);
	}

	// -------------------------------------------

	void TestVerification::TimeoutTest()
	{
		Utils::AtomicBool wasTimeout(false);
		int timeout = 1000;
		int runTime = timeout + 200;

        {
            TestVerification testObj(true, &wasTimeout, timeout);

            // dummi
            SingleTestDummi t;
            ISingleTest *test = &t; 
            TestResult res;      
            
            testObj.TestStarted(*test);

    		// make hang up
    		Platform::Thread::Sleep(runTime);
    
    		testObj.TestCompleted(*test, res);
        }
		ESS_ASSERT(wasTimeout);
	}

	// -------------------------------------------

	void TestVerification::SelfTests()
	{
        ThreadCountTest();
		TimeoutTest();
	}

} // namespace TestFw



