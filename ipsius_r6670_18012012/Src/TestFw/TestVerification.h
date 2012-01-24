
#ifndef __TESTVERIFICATION__
#define __TESTVERIFICATION__

// #include "ITestVerification.h"
#include "ITestFwOutputStream.h"

#include "Platform/PlatformTypes.h"
#include "Utils/AtomicTypes.h"
#include "Utils/ErrorsSubsystem.h"


namespace TestFw
{
	using Platform::dword;
    
    class TestLauncher;

    // -------------------------------------------

    // Verify that test was executed corectly: 
    // test finished in definite period of time,
    // no hung treads have been left
	class TestVerification 
		: QThread, 
			public ITestFwOutputStream 
	{
	// ITestFwOutputStream impl
	private:
        // do nothing
        void RunStarted(TestLauncher &launcher) {}
        void RunCompleted(TestLauncher &launcher) {}
        void GroupStarted(ITestGroup &group) {}
        void GroupCompleted(ITestGroup &group){}

        // check
        void TestStarted(ISingleTest &test);
        void TestCompleted(ISingleTest &test, const TestResult &result);
        
	private:
		bool m_ignoreTimeout;
        Utils::AtomicBool *m_pWasTimeout;
        dword m_timeout;
        dword m_threadCount;
        QMutex m_mutex;
		bool m_normalExit;
        
        void run();     // override

        void FinalCheck();
        void AssertTimeout();

		// self tests
		static void ThreadCountTest();
		static void TimeoutTest();

		// constructor for testing this class
        TestVerification(bool ignoreTimeout,
						 Utils::AtomicBool *pWasTimeout,
						 dword timeout = TestVerification::DefaultTimeout());
        
    public:

        // ESS_TYPEDEF(ThreadCountException);

        TestVerification(dword timeout = TestVerification::DefaultTimeout());
		~TestVerification();
        
        static dword DefaultTimeout();
		static void SelfTests();
	};

} // namespace TestFw


#endif
