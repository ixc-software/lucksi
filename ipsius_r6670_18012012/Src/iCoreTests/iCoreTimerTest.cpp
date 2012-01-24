
#include "stdafx.h"
#include "iCoreTimerTest.h"
#include "iCore/MsgTimer.h"
#include "iCore/MsgObject.h"
#include "iCore/ThreadRunner.h"
#include "Utils/AtomicTypes.h"


namespace 
{
    using namespace iCore;
    using Utils::AtomicBool;
    using Utils::AtomicInt;

    enum { CTimeInterval = 50, 
           CRepeatCount  = 8 };

    class TestParams : boost::noncopyable
    {
        int m_timeInterval;
        int m_repeatCount;
        int m_averange;

    public:
        TestParams() 
        {
            m_timeInterval = CTimeInterval;
            m_repeatCount = CRepeatCount;
            m_averange = 0;
        }

        int getTimeInterval() { return m_timeInterval; }
        int getRepeatCount()  { return m_repeatCount; }
        int getAverange()     { return m_averange; }

        void setAverange(int value)    { m_averange = value; }

        int getTimeout()
        {
            return m_repeatCount * m_timeInterval * 2;
        }

    };

    class MsgObjectTest : public MsgObject
    {
        typedef MsgObjectTest T;

        MsgTimer m_t;
        AtomicInt m_counter;
        AtomicBool &m_done;
        TestParams &m_params;
        SyncTimer m_timer;

        void OnTimer(MsgTimer *pT)
        {
            TUT_ASSERT(pT->getTag() == m_counter); 

            m_counter.Inc();
            pT->setTag(m_counter);

            if (m_counter < m_params.getRepeatCount()) return;

            // stop test
            m_params.setAverange(m_timer.Get() / m_counter);

            pT->Stop();
            pT->BindEvent(this, &T::OnFinish);
            pT->Start(10);
        }

        void OnFinish(MsgTimer *pT)
        {
            AsyncSetReadyForDeleteFlag(m_done);
        }

        void StartTimer()
        {
            m_t.Start(m_params.getTimeInterval(), true);
        }

    public:
        MsgObjectTest(MsgThread &thread, AtomicBool &done, TestParams &params) 
            : MsgObject(thread), 
            m_t(this, &T::OnTimer), m_counter(0), m_done(done), 
            m_params(params)
        {
            m_t.setTag(m_counter);
            StartTimer();
        }


    };

} // namespace

// ---------------------------------------------

namespace
{
    void OldStyleTest()
    {
        TestParams params;
        AtomicBool done;
        iCore::MsgThread m("OldStyleTest", Platform::Thread::LowPriority, true);
        MsgObjectTest obj(m, done, params);

        iCore::SyncTimer t;

        while(t.Get() < params.getTimeout())
        {
            Platform::Thread::Sleep(20);
            if (done.Get()) return; // ok
        }

        TUT_ASSERT(0 && "Timeout");
    }

} // namespace

// ----------------------------------------

namespace
{
    Utils::AtomicInt GDestructed;

    class TimerTest
    {        
        MsgObjectTest m_object;

    public:

        TimerTest(IThreadRunner &runner, TestParams &params) :
          m_object(runner.getThread(), runner.getCompletedFlag(), params)
        {
            ESS_ASSERT(runner.getThread().InCurrentThreadContext());
        }

        ~TimerTest()
        {
            GDestructed.Inc();
        }

    };

    void NewStyleTest(bool silenceMode)
    {
        GDestructed.Set(0);
        TestParams params;

        iCore::ThreadRunner test(Platform::Thread::LowPriority, params.getTimeout());

        if (test.Run<TimerTest>("TimerTest", params))
        {
            TUT_ASSERT(GDestructed.Get() == 1);

            if (!silenceMode) 
            {
                std::cout << "Avg. " << params.getAverange() << std::endl;
            }
        }
        else
        {
            TUT_ASSERT(0 && "Timeout");
        }

    }

} // namespace

// ----------------------------------------

namespace iCoreTests
{
    void iCoreTimerTest()
    {
        NewStyleTest(true);
    
        // OldStyleTest();
    }

} // namespace iCoreTests
