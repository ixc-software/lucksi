#include "stdafx.h"
#include "iCore/ThreadRunner.h"
#include "iCore/MsgObject.h"
#include "iCore/MsgTimer.h"
#include "iCoreExceptionHooks.h"

// ---------------------------------------------

namespace
{
    using namespace iCore;

    enum TestMode
    {
        ModeTestOK,
        ModeTestTimeout,
        ModeConstructorThrow,
        ModeMethodThrow,
        ModeThreadPoolThrow,
        ModeThreadStop
    };

    class TestParams
    {
        TestMode m_mode;

    public:
        TestParams(TestMode mode) : m_mode(mode) {}

        TestMode getMode() const { return m_mode; }
    };


    class TestRunner : public MsgObject
    {
    public:

        ESS_TYPEDEF(CreateFail);
        ESS_TYPEDEF(ThrowFromMethod);
        ESS_TYPEDEF(ThrowFromThreadPool);

        TestRunner(IThreadRunner &runner, TestParams &params)
            : MsgObject(runner.getThread()), 
            m_runner(runner), m_timer(this, &TestRunner::CompleteTestByTimer)
        {
            TestMode mode = params.getMode();

            if (mode == ModeTestOK)
            {
                CompleteTest();
                return;
            }

            if (mode == ModeTestTimeout)
            {
                return;
            }

            if (mode == ModeConstructorThrow)
            {
                ESS_THROW(CreateFail);
            }

            if (mode == ModeMethodThrow)
            {
                PutMsg(this, &TestRunner::MethodThrow);
                return;
            }

            if (mode == ModeThreadPoolThrow)
            {
                PutTask(this, &TestRunner::TaskBody);
                return;
            }

            if (mode == ModeThreadStop)
            {
                getMsgThread().Break();
                return;
            }

            ESS_ASSERT(0 && "Bad case!");
        }

    private:

        IThreadRunner &m_runner;
        MsgTimer m_timer;

        void CompleteTest()
        {
            AsyncSetReadyForDeleteFlag(m_runner.getCompletedFlag());
        }

        void CompleteTestByTimer(MsgTimer *pT)
        {
            CompleteTest();
        }


        void MethodThrow()
        {
            PutMsg(this, &TestRunner::CompleteTest);
            ESS_THROW(ThrowFromMethod);
        }

        void TaskBody()
        {
            m_timer.Start(500);
            ESS_THROW(ThrowFromThreadPool);
        }

    };

    // ----------------------------------------------------------

    class RunEvents : public iCore::IThreadRunnerEvents
    {
        ThreadRunner &m_threadRunner;
        TestMode m_mode;
        bool m_testOK;
        bool m_testFail;
        bool m_createException;

    // iCore::IThreadRunnerEvents implementation
    private:
        void OnCreateObjectFail(IThreadRunner &sender) 
        {
            ESS_ASSERT(m_threadRunner.IsSame(sender));

            if (m_mode == ModeConstructorThrow)
            {
                if ((!m_testOK) && m_createException)
                {
                    m_testOK = true;
                    return;
                }
            }

            m_testFail = true;
        }

        void OnTimeout(IThreadRunner &sender) 
        {
            if (m_mode == ModeTestTimeout)
            {
                if (!m_testOK)
                {
                    m_testOK = true;
                    return;
                }
            }

            m_testFail = true;
        }

        void OnThreadUnexpectedStop(IThreadRunner &sender)
        {
            if (m_mode == ModeThreadStop)
            {
                if (!m_testOK)
                {
                    m_testOK = true;
                    return;
                }
            }

            m_testFail = true;
        }


        void OnThreadException(IThreadRunner &sender, 
            const std::exception *pE, bool *pSuppressIt)
        {
            bool eCreate = (dynamic_cast<const TestRunner::CreateFail*>(pE) != 0);
            bool eMethod = (dynamic_cast<const TestRunner::ThrowFromMethod*>(pE) != 0);

            *pSuppressIt = true;

            if (eCreate)
            {
                if (m_mode == ModeConstructorThrow) m_createException = true;
                                                else m_testFail = true;
                return;
            }

            if (eMethod)
            {
                if ((m_mode == ModeMethodThrow) && !m_testOK) m_testOK = true;
                                                         else m_testFail = true;
                return;
            }

        }

        void OnThreadPoolException(IThreadRunner &sender, 
            const std::exception *pE, bool *pSuppressIt) 
        {
            *pSuppressIt = true;

            if (m_mode == ModeThreadPoolThrow)
            {
                if (!m_testOK)
                {
                    m_testOK = true;
                    return;
                }
            }

            m_testFail = true;
        }

    public:

        RunEvents(TestMode mode, ThreadRunner &threadRunner) 
            : m_threadRunner(threadRunner), m_mode(mode), 
            m_testOK(false), m_testFail(false), m_createException(false)
        {
            if (m_mode == ModeTestOK) m_testOK = true;
        }

        bool ResultOK()
        {
            return (m_testOK) && (!m_testFail);
        }
    };

    // ----------------------------------------------------------

    void RunTest(TestMode mode)
    {
        enum { CTimeout = 2 * 1000 };
        enum { CTimeoutShort = 100 };

        TestParams params(mode);

        // int timeout = (mode == ModeTestTimeout) ? CTimeoutShort : CTimeout;
        int timeout = CTimeout;
        if (mode == ModeTestTimeout) timeout = CTimeoutShort;
        
        iCore::ThreadRunner test(Platform::Thread::LowPriority, timeout);

        RunEvents events(mode, test);

        bool res = test.Run<TestRunner>("TestRunner", params, &events);

        // check
        TUT_ASSERT(events.ResultOK());

        if ((mode == ModeTestOK) || 
            (mode == ModeMethodThrow) || 
            (mode == ModeThreadPoolThrow))
        {
            TUT_ASSERT(res);
        }
        else
        {
            TUT_ASSERT(!res);
        }
    }

}  // namespace

// ---------------------------------------------

namespace iCoreTests
{
    void iCoreExceptionHooks()
    {    
        RunTest(ModeTestOK);
        RunTest(ModeTestTimeout);
        RunTest(ModeConstructorThrow); 
        RunTest(ModeMethodThrow);
        RunTest(ModeThreadPoolThrow);   
        RunTest(ModeThreadStop);
    }

} // namespace iCoreTests
