
#include "stdafx.h"

#include "iCoreMsgBenchmarks.h"

#include "Utils/ManagedList.h"
// #include "Utils/QtHelpers.h"
#include "iCore/MsgThread.h"
#include "iCore/MsgObject.h"
#include "iCore/SyncTimer.h"


namespace
{
    using namespace iCore;

    class MsgBench : public MsgObject
    {
        MsgBench *m_pOther;
        int m_counter;
        bool m_breaked;

        void OnSendEvent()
        {
            TUT_ASSERT(m_pOther);

            m_counter++;
            m_pOther->SendEvent();
        }

        void OnSendBreak()
        {
            getMsgThread().Break();
            m_breaked = true;
        }

    public:

        MsgBench(MsgThread &thread) : 
          MsgObject(thread), 
          m_pOther(0), m_counter(0), m_breaked(false)
        {
        }

        void setOther(MsgBench *pOther)
        {
            m_pOther = pOther;
        }

        void SendEvent()
        {
            if ((m_breaked) || (m_pOther->m_breaked)) return;
            if (!getMsgThread().IsRunning()) return;

            PutMsg(this, &MsgBench::OnSendEvent);
        }

        void ThreadBreak()
        {
            PutMsg(this, &MsgBench::OnSendBreak);
            getMsgThread().Wait();
        }

    };

    // -------------------------------------------------------

    std::string MakeResultString(const std::string &testName, int msTime, long msg)
    {
        double rate = (double(msg) * 1000.0 / msTime);	

        std::ostringstream ss;
        
        ss.precision(0);
        ss << testName << " " << std::fixed << rate << " msg/sec";
        ss << "; time " << msTime << ", msg " << msg;

        return ss.str();

        /*
        return QString("%1 %2 msg/sec; time %3 ms, msg %4")
            .arg(testName).arg(rate, 0, 'f', 0).arg(msTime).arg(msg); */
    }

    // --------------------------------------------------------

    int Wait(dword msTimeToRun)
    {
        SyncTimer t;
        while (t.Get() < msTimeToRun)
        {
            Platform::Thread::Sleep(100);
        }

        return t.Get();
    }

// --------------------------------------------------------

    class Benchmark
    {
        int m_threadsCount;
        int m_msTimeToRun;
        int m_msTimeToWait;
        long m_msgCount;
    
        class MsgSendingThread
        {
            MsgThread m_thread;
            MsgBench m_firstObj;
            MsgBench m_secondObj;
            long m_msgCount;

         public:

            MsgSendingThread() : 
               m_thread("MsgSendingThread", Platform::Thread::LowPriority, true),
               m_firstObj(m_thread), m_secondObj(m_thread), m_msgCount(0)
            {
                // m_thread.Run();
    
                m_firstObj.setOther(&m_secondObj);
                m_secondObj.setOther(&m_firstObj);
    
                //while(true) conversation between objects in the one thread
                m_firstObj.SendEvent();
            }
    
            void Break()
            {
                m_firstObj.ThreadBreak(); // m_thread.BreakAndWait();
                m_msgCount = m_thread.GetStats().MsgProcessed;
            }
    
            long getMsgCount()
            {
                return m_msgCount;
            }
        };
    
    public:
        Benchmark(int msTimeToRun, int threadsCount = 1)
            : m_threadsCount(threadsCount), m_msTimeToRun(msTimeToRun),
                m_msTimeToWait(0), m_msgCount(0)
        {};
    
        std::string Run()
        {
            Utils::ManagedList<MsgSendingThread> msgThreads;
            msgThreads.Reserve(m_threadsCount);
            
            // run m_threadsCount number of threads
            for (int i = 0; i < m_threadsCount; ++i)
            {
                msgThreads.Add(new MsgSendingThread());
            }
    
            //we are waiting - messages are sending
            m_msTimeToWait = Wait(m_msTimeToRun);
    
            //stops all threads and gets statistics
            for (int i = 0; i < m_threadsCount; ++i)
            {
                msgThreads[i]->Break();
                m_msgCount +=  msgThreads[i]->getMsgCount();
            }
    
            std::ostringstream testName;
            //testName << "Single thread benchmark (" << m_threadsCount << " threads)";
            testName << m_threadsCount 
                     << " threads benchmark (testing message sending in the framework of each thread) ";
    
            return MakeResultString(testName.str(), m_msTimeToWait, m_msgCount);
        }
    };
} // namespace

// --------------------------------------------------------

namespace iCoreTests
{
    std::string iCoreBenchmarkSingle(int msTimeToRun)
    {
        MsgThread m("MsgSendingThread", Platform::Thread::LowPriority);
    	m.Run();
    
    	MsgBench o1(m);
    	MsgBench o2(m);
    
    	o1.setOther(&o2);
    	o2.setOther(&o1);
    
    	o1.SendEvent();
    
    	int msTime = Wait(msTimeToRun);
    
    	o1.ThreadBreak(); // m.BreakAndWait();
    
    	return MakeResultString("Single thread benchmark", msTime, m.GetStats().MsgProcessed);
    }
    
    // --------------------------------------------------------
    
    std::string iCoreBenchmarkSingleTwoThread(int msTimeToRun)
    {
    	// thread one
        MsgThread m1("iCoreBenchmarkSingleTwoThread", Platform::Thread::LowPriority);
    	m1.Run();
    
    	MsgBench o1(m1);
    	MsgBench o2(m1);
    
    	o1.setOther(&o2);
    	o2.setOther(&o1);
    
    	o1.SendEvent();
    
    	// thread two
        MsgThread m2("iCoreBenchmarkSingleTwoThread", Platform::Thread::LowPriority);
    	m2.Run();
    
    	MsgBench o3(m2);
    	MsgBench o4(m2);
    
    	o3.setOther(&o4);
    	o4.setOther(&o3);
    
    	o3.SendEvent();
    
    	// wait
    	int msTime = Wait(msTimeToRun);
    
    	o1.ThreadBreak(); // m1.BreakAndWait();
    	o3.ThreadBreak(); // m2.BreakAndWait();
    
    	long msgCount = m1.GetStats().MsgProcessed + m2.GetStats().MsgProcessed;
    	return MakeResultString("Single thread benchmark (two threads)", msTime, msgCount);
    }
    
    
    // --------------------------------------------------------
    
    std::string iCoreBenchmarkMulti(int msTimeToRun)
    {
        MsgThread m1("iCoreBenchmarkMulti", Platform::Thread::LowPriority);
    	m1.Run();
        MsgThread m2("iCoreBenchmarkMulti", Platform::Thread::LowestPriority);
    	m2.Run();
    
    	MsgBench o1(m1);
    	MsgBench o2(m2);
    
    	o1.setOther(&o2);
    	o2.setOther(&o1);
    
    	o1.SendEvent();
    
    	int msTime = Wait(msTimeToRun);
    
    	o1.ThreadBreak(); // m1.BreakAndWait();
    	o2.ThreadBreak(); // m2.BreakAndWait();
    
    	//return MakeResultString("Multi thread benchmark", 
        return MakeResultString("2 threads benchmark (testing message sending between threads)",
            msTime, m1.GetStats().MsgProcessed + m2.GetStats().MsgProcessed);
    }
    
    // --------------------------------------------------------
    
    std::string iCoreBenchmarkSingleN(int msTimeToRun, int threadsCount)
    {
        Benchmark iCoreBenchmark(msTimeToRun, threadsCount);
        std::string results = iCoreBenchmark.Run();
    
        return results;
    
    }

} // namespace iCoreTests 
