
#include "stdafx.h"
#include "iCoreThreadPoolTest.h"
#include "iCore/MsgThread.h"
#include "iCore/MsgObject.h"
#include "iCore/MsgTimer.h"
#include "Utils/AtomicTypes.h"
#include "Utils/ManagedList.h"
#include "iCoreTests/TestUtils.h"


namespace
{
    using namespace iCore;

    enum
    {
        CAddTaskTime = 10,
        CTaskTime = 50,
        CTotalTask = 32,

        CTotalTestTime = CTaskTime * (CTotalTask) * 2,
    };

    class TestObject : public MsgObject
    {
        typedef TestObject T;

        MsgTimer m_timer;
        Utils::AtomicBool m_completed;
        int m_taskStarted;
        int m_taskCompleted;
        
        void OnTaskCompleted()
        {
            m_taskCompleted++;

            if (m_taskCompleted >= CTotalTask) 
            {
                m_completed.Set(true);
            }
        }

        void ThreadTask(long *p)
        {
            TUT_ASSERT(!m_completed);

            SyncTimer t;

            *p = 0;

            while(t.Get() < CTaskTime)
            {
                long val = *p;
                val++;
                *p = val;
            }

            PutMsg(this, &T::OnTaskCompleted);
        }

        void OnTimer(MsgTimer *pT)
        {
            if (m_taskStarted >= CTotalTask)
            {
                pT->Stop();
                return;
            }

            PutTask(this, &T::ThreadTask, new long);
            m_taskStarted++;
        }

    public:

        TestObject(MsgThread &mt) 
            : MsgObject(mt), m_timer(this, &T::OnTimer),
            m_taskStarted(0), m_taskCompleted(0)
        {
            m_timer.Start(CAddTaskTime, true);
        }

        bool Completed() { return m_completed; }

        // debug
        std::string GetResult()
        {
            std::stringstream ss;
            ss << m_taskStarted << "/" << m_completed << "\n";
            return ss.str();
        }

    };

    class Test : public TestUtils ::TestProcess
    {
        MsgThread m_mt;
        TestObject m_test;

        bool IsProcessed()  // override
        {
            return m_test.Completed();
        }

        bool TimeoutTestFail()  // override
        {
            //std::cout << m_test.GetResult();
            //return false;

            return true;
        }


    public:
        Test() : TestUtils::TestProcess(CTotalTestTime),
            m_mt("Test", Platform::Thread::LowPriority, true),
            m_test(m_mt)
        {
        }
    };

}  // namespace

// -------------------------------------------

namespace iCoreTests
{
    void iCoreThreadPoolTest()
    {
        Test test;
        test.Run();
    }

} // namespace iCoreTests
