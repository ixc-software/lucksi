
#ifndef __MSGTHREAD__
#define __MSGTHREAD__

#include "stdafx.h"
#include "Platform/Platform.h"
#include "Utils/ErrorsSubsystem.h"
#include "Utils/Random.h"
#include "Utils/AtomicTypes.h"
#include "Utils/SafeRef.h"
#include "Utils/VirtualInvoke.h"

#include "MsgBase.h"
#include "ICheckObject.h"
#include "TimerThread.h"
#include "MsgProcessor.h"
#include "ThreadPool.h"
#include "SyncTimer.h"

namespace iCore
{

    class MsgObject;
    class AsyncTasker;

	/*

    Очередь сообщений работающая в отдельном потоке.
    
		Данный класс занимается обработкой очереди сообщений в отдельном потоке
		Особенность класса -- максимальная эффективность (без использования
		мютексов) при работе в рамках одного потока

	*/
  
    class MsgThread : 
        Platform::Thread, 
        public ICheckGenerator, 
        public Utils::SafeRefServer
	{

	public:

        enum State
        {
            StIdle,
            StRunning,
            StStoped,
        };

        MsgThread(const std::string &name, 
            Platform::Thread::Priority priority = Platform::Thread::NormalPriority, 
            bool doRun = false) :  
            Platform::Thread(name),
            m_timers(m_timerUser.getThread()), 
            m_threadPool(priority)
		{
			Init(priority);
            if (doRun) Run();
		}

		~MsgThread()
		{
			// поток можно уничтожить только в том контексте, в котором он создан
            ESS_ASSERT(m_ownerThreadId == Platform::Thread::GetCurrentThreadID());

			BreakAndWait(); 
		}

		void Run()
		{
            enum { CWaitToRun = 5 * 1000 };

            ESS_ASSERT(m_state == StIdle);
			start(m_priority);

            // wait while thread really run
            SyncTimer t;

            while(true) 
            {
                if (t.Get() > CWaitToRun) ESS_ASSERT(0 && "Timeout");
                if ((IsRunning()) && (m_asyncTasker.get() != 0)) break;
                Platform::ThreadMinimalSleep();
            }
		}

		void PutMsg(MsgBase *msg);
        void PutTask(MsgBase *msg);

        TimerThread& GetTimerManager()
        {
            return m_timers;
        }

        // Статистика работы класса MsgThread
        struct Stats
        {
            long MsgProcessed;              // отработано сообщений
            long WakeUps;                   // число пробуждений потока
            int  MsgInQueue;                // текущее количество сообщений в очереди
            int  MsgObjectsDestroyed;       // кол-во удаленных MsgObject (через MsgObjectDestroyed)

            void Clear()
            {
                MsgProcessed = 0;
                WakeUps = 0;
                MsgInQueue = 0;
                MsgObjectsDestroyed = 0;
            }

            Stats()
            {
                Clear();
            }

            std::string ToString() const
            {
                std::ostringstream ss;

                ss << "MsgProcessed = "             << MsgProcessed
                    << "; WakeUps = "               << WakeUps 
                    << "; MsgInQueue = "            << MsgInQueue
                    << "; MsgObjectsDestroyed = "   << MsgObjectsDestroyed;

                return ss.str();

            }
        };

		// получение статистики из другого потока является потенциально небезопасным - !!
		Stats GetStats() const
		{
			if (IsRunning())
			{
				if (!InCurrentThreadContext()) ESS_ASSERT(m_sleeping);
			}

            Stats s = m_stats;
            s.MsgInQueue = m_quickQueue.size();

			return s;
		}

		bool getSleeping() const
		{
			return m_sleeping;
		}

		void BreakAndWait();
        void Break();
        void Wait();

        bool IsRunning() const
        {
            return (m_state == StRunning);
        }

        void MsgObjectDestroyed(MsgObject *pObject);

        bool InCurrentThreadContext() const
        {
            return (Platform::Thread::GetCurrentThreadID() == m_threadId);
        }

        // класс для описания кода, который нужно выполнить асинхронно в MsgThread
        class AsyncTask
        {
        public:
            virtual void Execute() = 0;
            virtual ~AsyncTask() {}
        };

        template<class T>
        class AsyncDeleteTask : public AsyncTask
        {
            T *m_p;

        public:

            AsyncDeleteTask(T *p) : m_p(p) {}

            void Execute()  // override
            {
                delete m_p;
            }
        };

        class VirtualInvokeTask : public AsyncTask
        {
            boost::scoped_ptr<Utils::IVirtualInvoke> m_invoke;

        public:

            VirtualInvokeTask(Utils::IVirtualInvoke *pInvoke)
            {
                ESS_ASSERT(pInvoke != 0);
                m_invoke.reset(pInvoke);
            }

            void Execute()  // override
            {
                ESS_ASSERT(m_invoke != 0);
                m_invoke->Execute();
                m_invoke.reset();
            }

        };

        void RunAsyncTask(boost::shared_ptr<AsyncTask> task);

        // T must be concrete type or have virtual ~T()
        template<class T>
        void AsyncDelete(T *p)
        {
            ESS_ASSERT(p != 0);

            boost::shared_ptr<AsyncTask> task( new AsyncDeleteTask<T>(p) );
            RunAsyncTask(task);
        }

        // pInvoke deleted inside
        void AsyncVirtualInvoke(Utils::IVirtualInvoke *pInvoke)
        {
            ESS_ASSERT(pInvoke != 0);

            boost::shared_ptr<AsyncTask> task( new VirtualInvokeTask(pInvoke) );
            RunAsyncTask(task);
        }

        ThreadPool& getThreadPool()
        {
            return m_threadPool;
        }

        void SetMaxMessageProcessingTimeMs(int interval)
        {
            m_timerUser.SetMaxMessageProcessingTimeMs(interval);
        }

        void GetMsgQueueDebugInfo(std::string &result, int msgLimit = 32);

        // MsgThread takes ownership
        void SetMsgProfiler(IMsgProcessorProfiler *pProfiler)
        {
            m_msgProfiler.reset(pProfiler);

            Utils::SafeRef<IMsgProcessorProfiler> ref;
            if (m_msgProfiler != 0) ref = m_msgProfiler.get();
            m_msgProcessor.SetProfiler(ref);
        }

        IMsgProcessorProfiler* MsgProfiler() 
        {
            return m_msgProfiler.get();
        }

        /*
        void SetErrorHook(Utils::SafeRef<IMsgProcessorErrorHook> hook)
        {
            m_msgProcessor.SetErrorHook(hook);
        } */

        void AddErrorHook(Utils::SafeRef<IMsgProcessorErrorHook> hook)
        {
            m_msgProcessor.AddErrorHook(hook);
        }

        void RemoveErrorHook(Utils::SafeRef<IMsgProcessorErrorHook> hook)
        {
            m_msgProcessor.RemoveErrorHook(hook);
        }
        
    protected:

        void run();  // override

    // IMsgProcessor impl
    private:

        MsgProcessor& GetMsgProcessor()
        {
            return m_msgProcessor;
        }

    // ICheckGenerator impl
    private:

        int GenerateCheckSignature() const
        {
            return m_random.Next();
        }

    private:

        typedef std::queue<MsgBase*> MsgQueue;

        TimerThreadUser m_timerUser;
        ThreadMsgCounterChecker m_msgCounterCheck;
        MsgQueue m_quickQueue;
        MsgQueue m_slowQueue;
        Platform::Thread::ThreadID m_threadId;
        Platform::Thread::ThreadID m_ownerThreadId;
        volatile bool m_break;
        volatile bool m_sleeping;
        volatile bool m_msgInSlowQueue;
        volatile State m_state;  
        Platform::Mutex m_mutex;
        Platform::WaitCondition m_msgToProcess;
        Priority m_priority;
        Stats m_stats;
        mutable Utils::Random m_random; 
        TimerThread &m_timers;
        boost::scoped_ptr<IMsgProcessorProfiler> m_msgProfiler;  // must be upper m_msgProcessor
        MsgProcessor m_msgProcessor;
        ThreadPool m_threadPool;
        boost::shared_ptr<AsyncTasker> m_asyncTasker;

        /* минимальное число сообщений, обрабатываемых в "быстрой" очереди
        перед переходом на чтение из "медленной" очереди */
        enum { CMinQuickQueuePoll = 4 };

        void MsgProcess(MsgBase *msg);
        void MsgLoop();
        void FreeAllMessagesInQueue();

        void Init(Priority priority);

        void WakeUp()
        {
            // if (InCurrentThreadContext()) return;
            // QMutexLocker lock(&m_mutex);
            // if (m_sleeping) 

            m_msgToProcess.wakeOne();
        }

        void RemoveMessagesForObject(MsgObject *pObject);
        void RemoveTimerForObject(MsgObject *pObject);
        void RemoveTesksForObject(MsgObject *pObject);

        static void MsgQueueCopy(MsgQueue &src, MsgQueue &dst);
	};


}



#endif

