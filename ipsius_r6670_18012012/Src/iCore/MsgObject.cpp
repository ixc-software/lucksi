
#include "stdafx.h"
#include "Utils/ThreadSyncEvent.h"
#include "MsgObject.h"

// ------------------------------------------------------

namespace iCore
{

    class ThreadSyncEvent : public Utils::ThreadSyncEvent {};

    // -------------------------------------------------------------

    void MsgObject::OnDestroy(boost::shared_ptr<ThreadSyncEvent> syncEvent)
    {
        m_destroyProcessed.Set(true);

        ESS_ASSERT(syncEvent.get() != 0);
        m_thread->MsgObjectDestroyed(this);

        syncEvent->Ready();
    }

    // -------------------------------------------------------------

    MsgObject::~MsgObject()
    {
        enum { CMaxWaitDestroy = 10 * 1000};

        // безопасно удал€ть себ€ нужно только если поток работает
        if (!m_thread->IsRunning()) return;

        // объект удал€ют из контекста m_thread, это можно сделать быстро и напр€мую
        if (m_thread->InCurrentThreadContext())
        {
            m_thread->MsgObjectDestroyed(this);
            return;
        }

        // объект удал€ют из контекста другого потока        
        if (m_disableOtherThreadSafeDelete.Get()) return;

        m_destroyRequested.Set(true);
        m_destroyProcessed.Set(false);

        boost::shared_ptr<ThreadSyncEvent> syncEvent( new ThreadSyncEvent() );

        AsyncExecuteDestroy(syncEvent); // PutMsg(this, &MsgObject::OnDestroy, syncEvent);

        if (!syncEvent->Wait(CMaxWaitDestroy))
        {
            bool state = m_thread->IsRunning(); // dummi
            ESS_ASSERT(0 && "Wait fail!");
        } 

        ESS_ASSERT(m_destroyProcessed.Get());
    }

    // -------------------------------------------------------------

    void MsgObject::AsyncDeleteSelf()
    {
        getMsgThread().AsyncDelete(this);
    }

    // -------------------------------------------------------------

    class AsyncSetReadyForDeleteFlagTask : public MsgThread::AsyncTask
    {
        Utils::AtomicBool *m_flag;

    public:

        AsyncSetReadyForDeleteFlagTask(Utils::AtomicBool *flag) : m_flag(flag) {}

        void Execute()  // override
        {
            m_flag->Set(true);
        }
    };


    void MsgObject::AsyncSetReadyForDeleteFlag(Utils::AtomicBool &flag)
    {
        typedef AsyncSetReadyForDeleteFlagTask Task;
        boost::shared_ptr<Task> task(new Task(&flag));
        getMsgThread().RunAsyncTask(task);
    }

    // -------------------------------------------------------------

    class AsyncExecuteDestroyTask : public MsgThread::AsyncTask
    {
        MsgObject *m_p;
        boost::shared_ptr<ThreadSyncEvent> m_syncEvent;

    public:

        AsyncExecuteDestroyTask(MsgObject *p, boost::shared_ptr<ThreadSyncEvent> syncEvent) : 
          m_p(p), m_syncEvent(syncEvent) {}

          void Execute()  // override
          {
              m_p->OnDestroy(m_syncEvent);
          }
    };

    void MsgObject::AsyncExecuteDestroy(boost::shared_ptr<ThreadSyncEvent> syncEvent)
    {
        typedef AsyncExecuteDestroyTask Task;
        boost::shared_ptr<Task> task(new Task(this, syncEvent));
        getMsgThread().RunAsyncTask(task);
    }


}  // namespace iCore
