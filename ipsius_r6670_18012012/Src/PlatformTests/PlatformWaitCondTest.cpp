#include "stdafx.h"

#include "Platform/Platform.h"

#include "Utils/TimerTicks.h"
#include "Utils/ErrorsSubsystem.h"
#include "Utils/ManagedList.h"
#include "Utils/Random.h"
#include "Utils/AtomicTypes.h"

#include "PlatformTests.h"

// -------------------------------------

namespace
{
    enum
    {
        CGenerateThreadCount = 4,
        CJobsPerThread = 16,
        CSleepPeriodMs = 10,
        CTotalJobs = CGenerateThreadCount * CJobsPerThread,
        CProcessTimeMs = 5,
    };

    // поток, отрабатывающий "задания" от GenerateThread
    class QueueThread : public Platform::Thread
    {
        Utils::Random m_rnd;
        Platform::Mutex m_mutex;
        Platform::WaitCondition m_wc;
        std::queue<int> m_jobs;
        int m_jobsProcessed;

        int PeekJob()
        {
            Platform::MutexLocker locker(m_mutex);

            while(true)
            {
                if (!m_jobs.empty()) 
                {
                    int job = m_jobs.front();
                    m_jobs.pop();
                    return job;
                }

                m_wc.wait(m_mutex);
            }
        }

        void ProcessJob(int job)
        {
            if (job > 0) Platform::ThreadSleep(job);

            m_jobsProcessed++;
        }

        void run()  // override
        {
            while(true)
            {
                int job = PeekJob();

                ProcessJob(job);

                if (m_jobsProcessed >= CTotalJobs) break;  // done
            }
        }

    public:

        QueueThread() : 
          Platform::Thread("QueueThread"),
          m_rnd( Platform::GetSystemTickCount() ), 
          m_jobsProcessed(0)
        {
            start(LowestPriority);
        }

        void PutJob(int jobTime)
        {
            Platform::MutexLocker locker(m_mutex);

            m_jobs.push(jobTime);

            m_wc.wakeOne();
        }

    };

    // ---------------------------------------------------------------

    // поток, создающий задания для QueueThread
    // у этих потоков приоритет выше, для более агрессивного прерывания работы QueueThread
    class GenerateThread : public Platform::Thread
    {
        QueueThread &m_thread;
        Utils::Random m_rnd;

        void run()  // override
        {
            int count = CJobsPerThread;

            while(count--)
            {
                // sleep
                Platform::ThreadSleep(1 + m_rnd.Next(CSleepPeriodMs));

                // add job
                int jobTime = 0;
                if (m_rnd.Next(2) != 0) m_rnd.Next(1 + CProcessTimeMs);

                m_thread.PutJob(jobTime);
            }
        }

    public:

        GenerateThread(QueueThread &thread, int seed) : 
          Platform::Thread("GenerateThread"),
          m_thread(thread), m_rnd(seed)
        {
            start(LowPriority);
        }
    };

    // ---------------------------------------------------------------

    void Test()
    {
        QueueThread mainThread;
        Utils::ManagedList<GenerateThread> threads;

        for(int i = 0; i < CGenerateThreadCount; ++i)
        {
            threads.Add( new GenerateThread(mainThread, i * i) );
        }

        mainThread.wait();
    }


}  


// -------------------------------------

namespace PlatformTests
{

	void WaitConditionTest()
	{
        Test();
	}
	
		
}  // namespace PlatformTests





