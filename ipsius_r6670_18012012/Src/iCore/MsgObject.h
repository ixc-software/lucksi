
#ifndef __MSGOBJECT__
#define __MSGOBJECT__

#include "stdafx.h"

#include "Utils/ErrorsSubsystem.h"
#include "Utils/SafeRef.h"
#include "Utils/AtomicTypes.h"

#include "MsgTemplates.h"
#include "MsgThread.h"
#include "ICheckObject.h"

namespace iCore
{
    class ThreadSyncEvent;

	/*
			Базовый тип для объекта, умеющего получать сообщения

	*/
    class MsgObject : public MsgObjectBase	
    {
        Utils::SafeRef<MsgThread> m_thread;
        Utils::AtomicBool m_destroyRequested;
        Utils::AtomicBool m_destroyProcessed;
        Utils::AtomicBool m_disableOtherThreadSafeDelete;

        void AssertNotDestroyed()
        {
            ESS_ASSERT(!m_destroyProcessed.Get());
        }

        void AssertNotDestroyReq()
        {
            ESS_ASSERT(!m_destroyRequested.Get());
        }

        void PutMsgToThread(MsgBase *pMsg) 
        {
            AssertNotDestroyReq();
            AssertNotDestroyed();
            ESS_ASSERT(pMsg);

            // put msg to queue
            m_thread->PutMsg(pMsg);
        }

        void DoPutTask(MsgBase *pMsg)
        {
            AssertNotDestroyReq();
            AssertNotDestroyed();
            ESS_ASSERT(pMsg);

            m_thread->PutTask(pMsg);
        }

        void AsyncExecuteDestroy(boost::shared_ptr<ThreadSyncEvent> syncEvent);

        friend class AsyncExecuteDestroyTask;
        void OnDestroy(boost::shared_ptr<ThreadSyncEvent> syncEvent);

		template<class Functor>
		void Function(Functor fn)
		{
			fn();
		}

	protected:

        template<class T1, class T2>
        MsgBase* CreateMsg(T1 owner, T2 fn)
        {
            return new MsgSimple<T1, T2>(owner, fn);
        }

        template<class T1, class T2, class T3>
        MsgBase* CreateMsg(T1 owner, T2 fn, T3 param, bool keepParam = false)
        {
            return new MsgParam<T1, T2, T3>(owner, fn, param, keepParam);
        }

		template<class Functor>
		MsgBase* CreateFunctorMsg(Functor fn)
		{
			return CreateMsg(this, &MsgObject::Function<Functor>, fn, true);
		}

		template<class Functor>
		void PutFunctor(Functor fn)
		{
			PutMsgToThread(CreateFunctorMsg(fn));
		}

		template<class T1, class T2>
		void PutMsg(T1 owner, T2 fn)
		{
			PutMsgToThread(CreateMsg(owner, fn));		
		}

		template<class T1, class T2, class T3>
		void PutMsg(T1 owner, T2 fn, T3 param, bool keepParam = false)
		{
			PutMsgToThread(CreateMsg(owner, fn, param, keepParam));		
		}

        template<class T1, class T2>
        void PutTask(T1 owner, T2 fn)
        {
            DoPutTask(CreateMsg(owner, fn));		
        }

        template<class T1, class T2, class T3>
        void PutTask(T1 owner, T2 fn, T3 param, bool keepParam = false)
        {
            DoPutTask(CreateMsg(owner, fn, param, keepParam));		
        }

        void AsyncDeleteSelf();
        void AsyncSetReadyForDeleteFlag(Utils::AtomicBool &flag);

	public:

		MsgObject(MsgThread &thread) : 
          MsgObjectBase(thread), 
          m_destroyRequested(false), m_destroyProcessed(false)
		{
            m_thread = Utils::SafeRef<MsgThread>(&thread);

            ESS_ASSERT(m_thread->IsRunning());
		}

        virtual ~MsgObject();

        MsgThread& getMsgThread() { return *(m_thread.operator->()); }

        // override
        bool getDestroyRequested() const { return m_destroyRequested.Get(); }

        // только для тестов -- это опасный трюк! 
        // впрочем, как и удаление объектов из другого потока
        void DisableOtherThreadSafeDelete() { m_disableOtherThreadSafeDelete.Set(true); }

        void PutMsgInQueue(MsgBase *pMsg)  // override
        {
            PutMsgToThread(pMsg);
        }

        ITimerManager& GetTimerManager()  // override
        {
            return m_thread->GetTimerManager();
        }

	};


}

#endif

