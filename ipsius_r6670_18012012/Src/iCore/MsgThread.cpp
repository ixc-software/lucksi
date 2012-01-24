#include "stdafx.h"
#include "MsgThread.h"
#include "MsgObject.h"
#include "Utils/AtomicTypes.h"

using Platform::MutexLocker;

namespace iCore
{

    class AsyncTasker : public MsgObject
    {

        void DoExecute(boost::shared_ptr<MsgThread::AsyncTask> task)
        {
            task->Execute();
        }

    public:

        AsyncTasker(MsgThread &thread) : MsgObject(thread)
        {
        }

        void Execute(boost::shared_ptr<MsgThread::AsyncTask> task)
        {
            PutMsg(this, &AsyncTasker::DoExecute, task);
        }

    };
    
    // -----------------------------------------------------------

    void MsgThread::Init(Priority priority)
    {
        m_ownerThreadId = Platform::Thread::GetCurrentThreadID();

        m_state = StIdle;
        m_priority = priority;
        m_threadId = 0;
        m_break = false;
        m_sleeping = false;
        m_msgInSlowQueue = false;
    }

    // -----------------------------------------------------------

    void MsgThread::RunAsyncTask(boost::shared_ptr<AsyncTask> task)
    {
        ESS_ASSERT(task.get() != 0);
        ESS_ASSERT(m_asyncTasker.get() != 0);

        m_asyncTasker->Execute(task);
    }

    // -----------------------------------------------------------

    void MsgThread::run()  // override
    {
        m_threadId = Platform::Thread::GetCurrentThreadID();

        m_state = StRunning; // m_running.Set(1);

        m_asyncTasker.reset( new AsyncTasker(*this) );

        try
        {
            MsgLoop();
            m_state = StStoped;  // тут нет однозначной синхронизации против PutMsg(), это потенциальная проблема
            FreeAllMessagesInQueue();  // возможно, не самое лучшее решение
        }
        catch (/*const*/ std::exception& e)
        {
            ESS_UNEXPECTED_EXCEPTION(e);
        }
        catch (...)
        {
            ESS_HALT("Unexpected unknown type exception");
        }

        m_state = StStoped;
    }


	// -----------------------------------------------------------

	void MsgThread::PutMsg(MsgBase *msg)
	{
        // эта штука редко, но может встречаться, см. примечание в MsgThread::run() 
        // не исправляю по причине того, что разрушение MsgThread на фоне обмена сообщениями это 
        // а) не правильно, б) встречается очень редко
        ESS_ASSERT(IsRunning());

		// put to 'quick' queue if we are in same thread
		if (InCurrentThreadContext())
		{            
			m_quickQueue.push(msg);
			return;
		}

		// write slow
		{
			MutexLocker lock(m_mutex);

			m_slowQueue.push(msg);
			m_msgInSlowQueue = true;

			// if (m_sleeping) 
            WakeUp();
		}

	}

    // -----------------------------------------------------------

    void MsgThread::PutTask(MsgBase *msg)
    {
        ESS_ASSERT(IsRunning());

        m_threadPool.Schedule(msg);
    }

	// -----------------------------------------------------------

	void MsgThread::MsgLoop()
	{
		while(!m_break)
		{
			int quickCounter = 0;

			// process quick queue
			while(!m_quickQueue.empty())
			{
				if (m_break) break;

				MsgBase *msg = m_quickQueue.front();
				m_quickQueue.pop();

				MsgProcess(msg);
				quickCounter++;

				if ((m_msgInSlowQueue) && (quickCounter >= CMinQuickQueuePoll)) break;
			}

			if (m_break) break;

			// process slow queue
			{
				MutexLocker lock(m_mutex);
				
				// copy slow -> quick
				if (!m_slowQueue.empty()) 
				{
					MsgQueueCopy(m_slowQueue, m_quickQueue);
					m_msgInSlowQueue = false;
					continue;
				}

				// goto sleep
				if (m_quickQueue.empty())
				{
					m_sleeping = true;
                    m_timerUser.CounterClear();
					m_msgToProcess.wait(m_mutex);
                    m_sleeping = false;
					m_stats.WakeUps++;
				}

			}
		}

	}

    // -----------------------------------------------------------

    void MsgThread::FreeAllMessagesInQueue()
    {
        MutexLocker lock(m_mutex);
        MsgQueueCopy(m_slowQueue, m_quickQueue);

        while(!m_quickQueue.empty())
        {
            MsgBase *p = m_quickQueue.front();
            m_quickQueue.pop();

            ESS_ASSERT(p);
            delete p;
        }
    }


	// -----------------------------------------------------------


	void MsgThread::MsgProcess(MsgBase *msg)
	{
        m_timerUser.CounterNext(msg);

        m_msgProcessor.DoProcessMsg(msg);

        m_timerUser.CounterNext(0);  // clear pointer to msg

        m_stats.MsgProcessed++;
	}

	// -----------------------------------------------------------

	void MsgThread::MsgQueueCopy(MsgQueue &src, MsgQueue &dst)
	{
		while(!src.empty())
		{
			dst.push(src.front());
			src.pop();
		}
	}

    // -----------------------------------------------------------

    void MsgThread::RemoveMessagesForObject(MsgObject *pObject)
    {
        // copy messages to quick queue
        {
            MutexLocker lock(m_mutex);
            MsgQueueCopy(m_slowQueue, m_quickQueue);            
        }

        // просматриваются все элементы очереди, если сообщение не предназначалось
        // для pObject, то оно просто ложиться обратно, в конец очереди
        int size = m_quickQueue.size();

        for(int i = 0; i < size; ++i)
        {
            MsgBase *p = m_quickQueue.front();
            m_quickQueue.pop();

            ESS_ASSERT(p);

            if (p->IsDestinationTo(pObject)) delete p;
                                        else m_quickQueue.push(p);
        }
    }

    // -----------------------------------------------------------

    void MsgThread::RemoveTimerForObject(MsgObject *pObject)
    {
        m_timers.RemoveTimersFor(pObject);
    }

    // -----------------------------------------------------------

    void MsgThread::RemoveTesksForObject(MsgObject *pObject)
    {
        m_threadPool.RemoveTasksFor(pObject);
    }

    // -----------------------------------------------------------

    void MsgThread::MsgObjectDestroyed(MsgObject *pObject)
    {
        // удалять MsgObject можно только в контексте потока, т.к. быстрая
        // очередь не синхронизируется, и к ней иначе нельзя получить безопасный доступ
        ESS_ASSERT(InCurrentThreadContext());

        {
            MsgBase *p = m_msgProcessor.Current();
            if (p != 0)
            {
                if (p->IsDestinationTo(pObject)) ESS_ASSERT(0 && "Delete self!");
            }
        }

        m_stats.MsgObjectsDestroyed++;

        RemoveMessagesForObject(pObject);
        RemoveTimerForObject(pObject);
        RemoveTesksForObject(pObject);
    }

    // -----------------------------------------------------------

    void MsgThread::Break()
    {
        if (!IsRunning()) return;

        m_break = true; 

        if (InCurrentThreadContext()) return;

        MutexLocker lock(m_mutex);
        WakeUp();
    }

    // -----------------------------------------------------------

    void DoFinalizeThreads();

    void MsgThread::Wait()
    {
        enum { CMaxWaitThreadTime = 10 * 1000};

        ESS_ASSERT(!InCurrentThreadContext());

        bool wasRunnig = Platform::Thread::isRunning();
        SyncTimer t;

        while( Platform::Thread::isRunning() )
        {
            DoFinalizeThreads();
            if (t.Get() > CMaxWaitThreadTime) ESS_ASSERT(0 && "Timeout");
            Sleep(50);
            Break(); 
        }

        if (wasRunnig) ESS_ASSERT(m_state == StStoped);

        // Старая версия кода, здесь был висяк, такое впечатление, 
        // что не срабатывал с первого раза Break()
        // if (QThread::isRunning()) QThread::wait();
    }

    // -----------------------------------------------------------

    void MsgThread::BreakAndWait()
    {
        if (!IsRunning()) return;

        Break();
        Wait();
    }

    // -----------------------------------------------------------

    void MsgThread::GetMsgQueueDebugInfo(std::string &result, int msgLimit)
    {
        ESS_ASSERT(msgLimit > 0);

        if ((msgLimit & 1) != 0) msgLimit++;

        // copy all data to temporary queue
        MsgQueue q;
        MsgQueueCopy(m_quickQueue, q);

        // add header
        std::ostringstream oss;
        oss << "Total messages in quick queue " << q.size() << "\n";
        oss << "List, from head to tail:\n";

        // process 
        int indexForIncludeFromHead = msgLimit / 2 - 1;
        int indexForAddElipsis = (msgLimit < q.size()) ? indexForIncludeFromHead + 1 : -1;
        int indexForIncludeFromTail = q.size() - msgLimit / 2;

        int index = 0;
        while(!q.empty())
        {
            bool include = (index <= indexForIncludeFromHead) || 
                           (index >= indexForIncludeFromTail);
            bool elipsis = (index == indexForAddElipsis);

            if (elipsis)
            {
                oss << "(...)\n";
            }

            MsgBase *p = q.front();
            q.pop();

            if (include)
            {
                oss << index << ": " << typeid(*p).name() << "\n";
            }

            index++;
        }

        // result
        result = oss.str();
    }



}  // namespace

