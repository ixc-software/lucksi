
#include "stdafx.h"
#include "ThreadPool.h"
#include "MsgObject.h"

// ----------------------------------------------------------------

namespace iCore
{

    // ѕоток объекта ThreadPool
    class ThreadPoolThread : Platform::Thread
    {
        ThreadPool &m_owner;
        Utils::AtomicBool m_break;
        Platform::WaitCondition m_wait;
        Platform::Mutex m_mutex;
        MsgBase *m_pTask;
        long m_taskCompleted;
        MsgProcessor m_msgProcessor;

        MsgBase* PeekTask()
        {
            MsgBase *pT = m_pTask;
            m_pTask = 0;

            return (pT != 0) ? (pT) : (m_owner.PeekTask());  
        }

        void ProcessTask()
        {
            while(true)
            {
                MsgBase *pT = PeekTask();
                if (pT == 0) break;

                m_msgProcessor.DoProcessMsg(pT);
                m_taskCompleted++;
            }

        }

        void ThreadLoop()
        {
            Platform::MutexLocker locker(m_mutex);

            ProcessTask();

            m_wait.wait(m_mutex);  // wait
        }

        void run()  // override
        {
            try
            {
                while(!m_break.Get())
                {
                    ThreadLoop();
                }

                ESS_ASSERT(m_pTask == 0);
            }
            catch(std::exception &e)
            {
                ESS_UNEXPECTED_EXCEPTION(e);
            }
        }

        void WakeUp()
        {
            m_wait.wakeOne();
        }

    public:

        ThreadPoolThread(ThreadPool &owner, Platform::Thread::Priority priority) : 
          Platform::Thread("ThreadPoolThread"),
          m_owner(owner), m_pTask(0), m_taskCompleted(0)
        {
            start(priority);
        }

        ~ThreadPoolThread()
        {
            // может быть ненадежным!
            m_break.Set(true);
            WakeUp();

            wait();
        }

        bool TryPutTask(MsgBase *pTask)
        {
            ESS_ASSERT(pTask != 0);
            bool placeTask = false;

            if (!m_mutex.TryLock()) return false;

            if (m_pTask == 0) 
            {
                m_pTask = pTask;
                placeTask = true;
            }

            WakeUp();

            m_mutex.Unlock();

            return placeTask;
        }

        bool ActiveFor(MsgObject *pObject)
        {
            /* 
              Ќе реализовано -- трудно добитьс€ синхронизации.

              Ёта проверка перестраховочна€, должна работать проверка через SafeRef<> в MsgBase

              ќдин из вариантов реализации -- получать из MsgBase голый указатель на MsgObject
              перед запуской задачи, и обнул€ть его после отработки задачи
              “ут этот указатель можно было бы сравнить с передаваемым
            */ 
            return false;
        }

        long getTaskCompleted() { return m_taskCompleted; }

        void SetErrorHook(Utils::SafeRef<IMsgProcessorErrorHook> hook)
        {
            if ( !hook.IsEmpty() )
            {
                m_msgProcessor.AddErrorHook(hook);
            }
        }

    };   // class ThreadPoolThread


    // -----------------------------------------------------------


    MsgBase* ThreadPool::PeekTask()
    {
        Utils::Locker lock(m_tasks);

        if (m_tasks.empty()) return 0;

        MsgBase *pTask = m_tasks.front();
        m_tasks.pop();

        return pTask;
    }

    // -----------------------------------------------------------

    bool ThreadPool::PlaceTaskToThread(MsgBase *pT)
    {
        // try to place on created threads
        for(int i = 0; i < m_threads.size(); i++)
        {
            if (m_threads[i]->TryPutTask(pT)) return true;
        }

        // try to place on new thread
        if (m_threads.size() >= m_maxThreads) return false;

        Thread *pThread = new Thread(*this, m_priority);
        pThread->SetErrorHook(m_defaultHook);
        m_threads.push_back(pThread);

        return (pThread->TryPutTask(pT));
    }

    // -----------------------------------------------------------

    void ThreadPool::TryPeekTask() 
    {
        ESS_ASSERT(m_tasks.IsLockedByLocker());

        while(!m_tasks.empty())
        {
            if (PlaceTaskToThread(m_tasks.front())) 
            {
                m_tasks.pop();
                continue;
            }

            break;
        }

    }

    // -----------------------------------------------------------

    ThreadPool::~ThreadPool()
    {
        for(int i = 0; i < m_threads.size(); i++)
        {
            delete m_threads[i];
        }
    }

    // -----------------------------------------------------------

    void ThreadPool::Schedule(MsgBase *pTask)
    {
        Utils::Locker lock(m_tasks);

        ESS_ASSERT(pTask);

        m_tasks.push(pTask);

        TryPeekTask();
    }

    // -----------------------------------------------------------

    void ThreadPool::RemoveTasksFor(MsgObject *pObject)
    {
        Utils::Locker lock(m_tasks);  // protect m_tasks and m_threads

        // убрать задачи из очереди
        {
            int size = m_tasks.size();

            for(int i = 0; i < size; ++i)
            {
                MsgBase *p = m_tasks.front();
                m_tasks.pop();

                ESS_ASSERT(p);

                if (p->IsDestinationTo(pObject)) delete p;
                else m_tasks.push(p);
            }

        }

        /*  
            ”бедитьс€, что удал€емый объект не имеет выполн€емых задач.
        Ќаличие выполн€емой задачи фатально, т.к. данный код выполн€етс€ из
        деструктора MsgObject, т.е. деструктор типа, который создавал задачу,
        уже отработал, а код в его теле еще продолжает выполн€тьс€
        */
        {
            for(int i = 0; i < m_threads.size(); i++)
            {                
                if (m_threads[i]->ActiveFor(pObject)) ESS_ASSERT(0 && "Fatal!");
            }

        }
    }

    // -----------------------------------------------------------

    void ThreadPool::SetErrorHook( Utils::SafeRef<IMsgProcessorErrorHook> hook )
    {
        Utils::Locker lock(m_tasks);

        // change for exists threads
        for(int i = 0; i < m_threads.size(); ++i)
        {
            m_threads.at(i)->SetErrorHook(hook);
        }

        // default hook
        m_defaultHook = hook;
    }

	
} // namespace iCore
