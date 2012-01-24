
#include "stdafx.h"

#include "iCoreTimerTestAdvanced.h"
#include "iCoreTimerTest.h"

#include "iCore/MsgTimer.h"
#include "iCore/TimerThread.h"
#include "iCore/SyncTimer.h"
#include "iCore/MsgThread.h"
#include "iCore/MsgObject.h"

#include "Utils/AtomicTypes.h"
#include "Utils/ManagedList.h"
#include "iCoreTests/TestUtils.h"

namespace
{
    using namespace iCore;
    using namespace Utils;
    using namespace TestUtils;

    enum 
    { 
        CTimeInterval = 50, 
        CEventCount = 4,
        CDuration = CTimeInterval * CEventCount * 2,

        CThreadCount = 10,
        CObjPerThreadCount = 2,
    };

    enum  
    { 
        CEVENTSTART = 100, 
        CEVENT1 = 101, 
        CEVENT2 = 102, 
        CEVENTFINISH = 199, 
    };


    class TimerManipulator : public MsgObject
    {
        typedef TimerManipulator owner;

        MsgTimer m_timer;
        AtomicBool m_start;
        AtomicBool m_finish;
        AtomicInt m_val;
        Random m_random;

        //set timer new tag (test tags)
        void ChangeTag(int oldTag, int newTag)
        {
            TUT_ASSERT(m_timer.getTag() == oldTag);

            m_timer.ClearTag();
            m_timer.setTag(newTag);
        }

        void TimerStart()
        {
            dword halfTime = CTimeInterval / 2;
            m_timer.Start(m_random.Next(halfTime) + halfTime);
        }

        //timer events
        void OnTimerStart(MsgTimer *pT)
        {
            m_start.Set(true);

            m_timer.BindEvent(this, &owner::OnTimerEvent1);
            ChangeTag(CEVENTSTART, CEVENT1);

            TimerStart();
        }

        void OnTimerEvent1(MsgTimer *pT)
        {
            TUT_ASSERT(m_start);

            m_val.Set(CEVENT1);

            m_timer.BindEvent(this, &owner::OnTimerEvent2);
            ChangeTag(CEVENT1, CEVENT2);

            TimerStart();
        }

        void OnTimerEvent2(MsgTimer *pT)
        {
            TUT_ASSERT(m_val == CEVENT1);

            m_val.Set(CEVENT2);

            m_timer.BindEvent(this, &owner::OnTimerFinish);
            ChangeTag(CEVENT2, CEVENTFINISH);

            TimerStart();
        }

        void OnTimerFinish(MsgTimer *pT)
        {
            TUT_ASSERT(m_val == CEVENT2);

            // m_finish.Set(true);
            AsyncSetReadyForDeleteFlag(m_finish);
        }
        
    public:

        TimerManipulator(MsgThread& thread, int seed) 
            : MsgObject(thread), 
            m_timer(this, &owner::OnTimerStart), m_random(seed)
        {
            m_timer.setTag(CEVENTSTART);
            m_timer.Start(CTimeInterval);
        }

        bool IsProcessed()
        {
            return m_finish;
        }
    };

   // ----------------------------------------

    class TimerAdvancedTest : public TestProcess
    {
        class ThreadWithObjs
        {
            MsgThread m_thread;
            ManagedList<TimerManipulator> m_objs;
    
        public:
            ThreadWithObjs(Platform::Thread::Priority priority, int threadNo) 
                : m_thread("ThreadWithObjs", priority, true)
            {
                Random random(threadNo);

                m_objs.Reserve(CObjPerThreadCount);
                int i = CObjPerThreadCount;
                while (i--)
                {
                    m_objs.Add(new TimerManipulator(m_thread, random.Next()));
                }
            }

            bool IsProcessed()
            {
                for (size_t i = 0; i < m_objs.Size(); ++i)
                {
                    if (!m_objs[i]->IsProcessed()) return false;
                }

                return true;
            }
        };

        ManagedList<ThreadWithObjs> m_threads;

    protected:
        bool IsProcessed()
        {
            for (size_t i = 0; i < m_threads.Size(); ++i)
            {
                if (!m_threads[i]->IsProcessed()) return false;
            }

            return true;
        }

    public:
        TimerAdvancedTest()
            : TestProcess(CDuration)
        {
            Random random;
            m_threads.Reserve(CThreadCount);
            int i = CThreadCount;
            while (i--)
            {
                m_threads.Add( new ThreadWithObjs(Platform::Thread::LowPriority, random.Next()) );
            }
        }
    };

    // ----------------------------------------

    class TimerAdvancedSingleTest : public TestProcess
    {
        TimerManipulator m_timerManipulator;
        
    protected:
        bool IsProcessed()
        {
            return m_timerManipulator.IsProcessed();
        }
    public:
        TimerAdvancedSingleTest(dword testDuration, MsgThread& thread)
            : TestProcess(testDuration), m_timerManipulator(thread, 1)
        {}
    };


} // namespace

// ----------------------------------------

namespace iCoreTests
{
    void iCoreTimerAdvancedTest()
    {
        TimerAdvancedTest test;
        test.Run();
    }
    
    // ----------------------------------------
    
    //single thread
    void iCoreTimerAdvancedTest_()
    {
        MsgThread m("iCoreTimerAdvancedTest_", Platform::Thread::LowPriority, true);
        TimerAdvancedSingleTest test(CDuration, m);
        test.Run();
    }

} // namespace iCoreTests
