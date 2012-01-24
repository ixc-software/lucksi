#include "stdafx.h"
#include "Utils/ErrorsSubsystem.h"
#include "Utils/TimerTicks.h"
#include "iCore/MsgObject.h"
#include "iCore/MsgThreadMonitor.h"
#include "iCore/MsgTimer.h"
#include "Domain/DomainTestRunner.h"
#include "Domain/DomainClass.h"
#include "iCoreOverload.h"

using namespace Domain;
using namespace iCore;

namespace TestMsgHalt
{
    // debug switches
    enum
    {
        // set for view application crash with watchdog message
        CViewWatchdogWork = false,  

        // set for view crash by unexpected exception in TimerThread
        CViewUnknownExceptionInTimerThread = false,  

        // DomainTestRunner timeout crash -- works as ThreadRunner::AsyncDelete
        // timeout, 'couse message loop blocked by TestObject::Loop()
        CDomainTestRunnerTimeout = false,
    };

    enum
    {
        CMsgTimeout = 1000,
    };

    // ------------------------------------------------------

    ESS_TYPEDEF_FULL(DummiException, ESS::HookRethrow);

    class XHook : public ESS::ExceptionHook
    {
        bool m_captured;

        void Hook(const ESS::BaseException *pE)  // override
        {
            if (CViewWatchdogWork) return;  // debug
            if (m_captured) return;

            if (dynamic_cast<const ESS::Assertion*>(pE))
            {
                m_captured = true;

                std::string msg = "RETHROW " + pE->getTextAndPlace();

                if (CViewUnknownExceptionInTimerThread)  // debug
                {
                    ESS::DoException<DummiException>(msg, 0, 0, 0);
                }
                else
                {
                    ESS::DoException<TimerThread::SuppressWatchdogException>(msg, 0, 0, 0);
                }
            }
        }

    public:

        XHook() : m_captured(false)
        {
        }

        bool Captured() const
        {
            return m_captured;
        }
    
    };

    // ------------------------------------------------------

    class  TestParams
    {
    };

    class TestObject : public MsgObject
    {
        DomainClass &m_domain;
        XHook &m_hook;

        void Loop()
        {
            while(true)
            {
                if (!CDomainTestRunnerTimeout)
                {
                    if (m_hook.Captured()) 
                    {
                        break;
                    }
                }
                Platform::Thread::Sleep(50);
            }

            m_domain.Stop(Domain::DomainExitOk);
        }

    public:

        TestObject(DomainClass &domain, XHook &hook) : 
          MsgObject(domain.getMsgThread()),
          m_domain(domain),
          m_hook(hook)
        {
            m_domain.getMsgThread().SetMaxMessageProcessingTimeMs(CMsgTimeout);
            PutMsg(this, &TestObject::Loop);
        }
    };

    // ------------------------------------------------------

    class Test
    {
        XHook m_hook;
        TestObject m_object;

    public:

        Test(DomainClass &domain, TestParams &params)
            : m_object(domain, m_hook)
        {            
            // ...
        }
    
    };

    void Run()
    {
        DomainTestRunner runner(CMsgTimeout * 5);

        TestParams params;

        bool res = runner.Run<Test>(params);
        TUT_ASSERT(res);
    }

}  // namespace TestMsgHalt

// ----------------------------------------------------------------

namespace TestMonitor
{
    const int CTimeout      = 10 * 1000;
    const int CPollInterval = 100;
    const int CMaxLagValue  = 50;

    enum TestMode
    {
        tmMaxMsg,
        tmMaxTimerLag,
        tmMsgGrow,
    };
    
    struct TestParams
    {
        TestMode Mode;
    };


    // -----------------------------------------------------------------

    class Test : public MsgObject, public IMsgThreadMonitorCallback
    {
        DomainClass &m_domain;
        TestMode m_mode;
        MsgThreadMonitor m_monitor;
        Utils::AtomicBool m_stopMsgGrow;

        void KeepMsgCount()
        {
            PutMsg(this, &Test::KeepMsgCount);
        }

        void MaxMsgGenerate()
        {
            const int CMaxMsg = 64;

            m_monitor.SetMaxMsgQueueSize(CMaxMsg);

            int count = CMaxMsg * 2;

            while(count--) KeepMsgCount();

            // m_timer.Start(CPollInterval * 3);
        }

        void SlowMsg()
        {
            Utils::TimerTicks t;
            t.Sleep(CPollInterval + CMaxLagValue * 3);

            PutMsg(this, &Test::SlowMsg);
        }

        void StartTimerLag()
        {
            const int CRepeatLag = 2;

            m_monitor.SetMaxTimerLag(CRepeatLag, CMaxLagValue);

            SlowMsg();
        }

        void MsgProgressGrow()
        {
            if (m_stopMsgGrow) return;
            
            PutMsg(this, &Test::MsgProgressGrow);
            PutMsg(this, &Test::MsgProgressGrow);
        }

        void StartMsgGrow()
        {
            const int CGrowMinimalLimit = 10;

            m_monitor.SetMsgGrowDetection(2, CGrowMinimalLimit);

            int count = CGrowMinimalLimit;

            while(count--)
            {
                MsgProgressGrow();
            }
        }

    // IMsgThreadMonitorCallback impl
    private:

        void CallbackForException(const std::exception *pE, bool *pSuppressIt)
        {
            Domain::DomainExitCode code = Domain::DomainExitErr; 
            if (m_mode == tmMaxMsg)
            {
                if (dynamic_cast<const MsgThreadMonitor::MaxMessages*>(pE))
                {
                    *pSuppressIt = true;
                    m_domain.Stop(code);
                    return;
                } 
            }

            if (m_mode == tmMaxTimerLag)
            {
                if (dynamic_cast<const MsgThreadMonitor::TimerLag*>(pE))
                {
                    *pSuppressIt = true;
                    m_domain.Stop(code);
                    return;
                } 
            }

            if (m_mode == tmMsgGrow)
            {
                if (dynamic_cast<const MsgThreadMonitor::MsgGrow*>(pE))
                {
                    *pSuppressIt = true;
                    m_domain.Stop(code);
                    // abort msg senging recursion
                    m_stopMsgGrow.Set(true);
                    return;
                } 
            }

            ESS_ASSERT(0 && "Bad exception");
        }


    public:

        Test(DomainClass &domain, TestParams &params) : 
            MsgObject(domain.getMsgThread()), 
            m_domain(domain),
            m_mode(params.Mode),
            m_monitor(domain.getMsgThread(), CPollInterval, this)
        {
            domain.ThreadMonitor().Disable();

            if (m_mode == tmMaxMsg)      MaxMsgGenerate();
            if (m_mode == tmMaxTimerLag) StartTimerLag();
            if (m_mode == tmMsgGrow)     StartMsgGrow();
        }

    };

    // -----------------------------------------------------------------

    void Run(TestMode mode)
    {
        DomainTestRunner runner(CTimeout);

        TestParams params;
        params.Mode = mode;

        bool res = runner.Run<Test>(params);
        TUT_ASSERT(res);
    }

    void TestMaxMsgInThreadHalt()
    {
        Run(tmMaxMsg);
    }

    void TestMaxTimerLag()
    {
        Run(tmMaxTimerLag);
    }

    void TestMsgGrow()
    {
        Run(tmMsgGrow);
    }



}  // namespace TestMonitor


// ----------------------------------------------------------------

namespace iCoreTests
{
	
	void TestBlockedMsgHalt()
	{
        TestMsgHalt::Run();
	}

    void TestMaxMsgInThreadHalt()
    {
        TestMonitor::TestMaxMsgInThreadHalt();
    }

    void TestMaxTimerLag()
    {
        TestMonitor::TestMaxTimerLag();
    }

    void TestMsgGrow()
    {
        TestMonitor::TestMsgGrow();
    }
    
    template<class T>
    void DebugPrint<T>::Print(T val)
    {
        std::cout << val;
    }

    template class DebugPrint<int>;
	
}  // namespace iCoreTests
