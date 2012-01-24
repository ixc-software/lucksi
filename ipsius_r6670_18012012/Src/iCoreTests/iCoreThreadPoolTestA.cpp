
#include "stdafx.h"

#include "iCoreThreadPoolTestA.h"

#include "iCore/MsgThread.h"
#include "iCore/MsgObject.h"

#include "Utils/Random.h"
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
        CTaskCount = 50,
        CTaskDurationMS = 50,
        CTestDurationMS = CTaskCount * CTaskDurationMS * 2,
        CMaxDiffPercent = 50,
    };

    // ----------------------------------------

    bool InInterval(int val, int averageVal, int percent)
    {
        int maxVal = averageVal + (averageVal * percent / 100);
        int minVal = averageVal - (averageVal * percent / 100);
        
        return ((val < minVal) || (val > maxVal)) ? false : true;
    }

    // ----------------------------------------

    class ThreadPoolTest : public TestProcess
    {
        class TaskObject : public MsgObject
        {
            AtomicInt m_taskCounter;
            AtomicBool m_completed;
            // key = TID (thread ID), val = counter
            typedef std::map<Qt::HANDLE, int> Tasks;
            typedef Tasks::const_iterator CI;
            Tasks m_taskPerThread; 
            TestUtils::TestLog m_log;

            void IncrementTaskCounter()
            {
                m_taskCounter.Inc();

                Qt::HANDLE taskID = QThread::currentThreadId();
                
                m_taskPerThread[taskID] += 1;

                if (m_taskCounter >= CTaskCount) AsyncSetReadyForDeleteFlag(m_completed);
            }

            void OnNewTask()
            {
                SyncTimer t;
                while (t.Get() < CTaskDurationMS);

                // IncrementTaskCounter();
                PutMsg(this, &TaskObject::IncrementTaskCounter);
            }

        public:
            TaskObject(MsgThread &thread) 
                : MsgObject(thread), 
                    m_log("Thread pull test (A)")
            {
                for (int i = 0; i < CTaskCount; ++i)
                {
                    PutTask(this, &TaskObject::OnNewTask);
                }
            }

            void ShowLog()
            {
                m_log.Log("Tasks per thread allocation: ");

                for (CI p = m_taskPerThread.begin(); 
                      p != m_taskPerThread.end(); 
                      ++p)
                {
                    std::stringstream s;
                    s << "thread " << p->first << " : " << p->second;
                    m_log.Log(s.str());
                }

                m_log.WriteTo(std::cout);
            }

            bool IsComplited()
            {
                return m_completed.Get();
            }

            bool TaskAllocationIsNormal()
            {
                size_t threadCount = m_taskPerThread.size();
                TUT_ASSERT(threadCount != 0);

                // task per thread allocation 
                int taskPerThread = CTaskCount / threadCount; 

                for (CI p = m_taskPerThread.begin(); 
                      p != m_taskPerThread.end(); 
                      ++p)
                {
                    if (!InInterval(p->second, taskPerThread, CMaxDiffPercent)) 
                        return false;
                }
                return true;
            }
        };

        MsgThread m_thread;
        TaskObject m_obj;
        bool m_printInfo;

    protected:
        bool IsProcessed()
        {
            if (!m_obj.IsComplited()) return false;

            if (m_printInfo) m_obj.ShowLog();

            // TUT_ASSERT(m_obj.TaskAllocationIsNormal());

            return true;
        }

    public:
        ThreadPoolTest(bool printInfo)
            : TestProcess(CTestDurationMS), 
            m_thread("ThreadPoolTest", Platform::Thread::LowPriority, true), 
                m_obj(m_thread),
                m_printInfo(printInfo)
        {}
    };
} // namespace

// ----------------------------------------

namespace iCoreTests
{
    void iCoreThreadPoolTestA(bool printInfo)
    {
        ThreadPoolTest test(printInfo);
        test.Run();
    }

} // namespace iCoreTests
