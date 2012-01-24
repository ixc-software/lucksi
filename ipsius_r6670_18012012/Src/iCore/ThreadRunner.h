#ifndef __THREADRUNNER__
#define __THREADRUNNER__

#include "stdafx.h"

#include "Utils/IBasicInterface.h"
#include "Utils/AtomicTypes.h"
#include "Utils/SafeRef.h"

#include "MsgThread.h"
#include "SyncTimer.h"
#include "MsgObject.h"

namespace iCore
{
    // интерфейс, по которому создаваемый в ThreadRunner объект общается с ним
    class IThreadRunner : public Utils::IBasicInterface
    {
    public:
        virtual MsgThread& getThread() = 0;
        virtual Utils::AtomicBool& getCompletedFlag() = 0;
        virtual void ResetWatchDog() = 0;
    };

    class IThreadRunnerEvents : public Utils::IBasicInterface
    {
    public:
        virtual void OnCreateObjectFail(IThreadRunner &sender) = 0;
        virtual void OnTimeout(IThreadRunner &sender) = 0;
        virtual void OnThreadUnexpectedStop(IThreadRunner &sender) = 0;

        // this called from another thread context
        virtual void OnThreadException(IThreadRunner &sender, 
            const std::exception *pE, bool *pSuppressIt) = 0;
        virtual void OnThreadPoolException(IThreadRunner &sender, 
            const std::exception *pE, bool *pSuppressIt) = 0;
    };

    // ----------------------------------------------------------

    namespace TR
    {
        // реализация интефрейса IThreadRunner
        class Helper : public IThreadRunner
        {
            MsgThread &m_thread;
            Utils::AtomicBool m_completed;
            bool m_timerReset;

        // IThreadRunner implementation
        public:

            MsgThread& getThread() { return m_thread; }
            Utils::AtomicBool& getCompletedFlag() { return m_completed; }
            void ResetWatchDog() {  m_timerReset = true; }

        public:

            Helper(MsgThread &thread) : m_thread(thread), m_timerReset(false)
            {
            }

            bool PollTimerReset()
            {
                if (!m_timerReset) return false;

                m_timerReset = false;
                return true;
            }

            bool Completed()
            {
                return m_completed.Get();
            }

        };

        // ----------------------------------------

        // код синхронного создания объекта заданного типа
        template<class TObject, class TObjectParams>
        class Creator : public MsgThread::AsyncTask
        {
            TObject *m_pObject;
            TObjectParams &m_params;
            IThreadRunner &m_helper;
            Utils::AtomicBool m_wasException;

            void Execute()  // override
            {
                try
                {
                    m_pObject = new TObject(m_helper, m_params);
                }
                catch(...)
                {
                    m_wasException.Set(true);
                    throw;
                }
            }

        public:

            Creator(TObjectParams &params, IThreadRunner &helper)
                : m_pObject(0), m_params(params), m_helper(helper)
            {
            }

            TObject* Result()
            {
                return m_pObject;
            }

            bool WasException()
            {
                return m_wasException;
            }

        };

        // ----------------------------------------

        // код синхронного удаления объекта заданного типа
        template<class TObject>
        class Destructor : public MsgThread::AsyncTask
        {
            TObject *m_pObject;
            Utils::AtomicBool m_done;

            void Execute()  // override
            {
                delete m_pObject;
                m_done.Set(true);
            }

        public:

            Destructor(TObject *pObject) : m_pObject(pObject)
            {
                ESS_ASSERT(m_pObject);
            }

            bool Result()
            {
                return m_done.Get();
            }

        };

        // -----------------------------------------

        // перехватчик исключений
        class HookThread : 
            public virtual Utils::SafeRefServer,
            public IMsgProcessorErrorHook
        {
            IThreadRunner &m_sender;
            IThreadRunnerEvents *m_pEvents; 
            bool m_isThreadPool;

        // IMsgProcessorErrorHook impl
        private:

            void OnMsgException(const std::exception &e, MsgBase *pMsg, bool &suppressIt) 
            {
                if (m_pEvents == 0)
                {
                    std::ostringstream oss;
                    oss << "HookThread is null; " << e.what();
                    if (pMsg != 0)
                    {
                        oss << "; " << typeid(*pMsg).name();
                    }
                    ESS_HALT(oss.str());
                }

                if (m_isThreadPool) m_pEvents->OnThreadPoolException(m_sender, &e, &suppressIt);
                               else m_pEvents->OnThreadException(m_sender, &e, &suppressIt);
            }

        public:

            HookThread(IThreadRunner &sender, IThreadRunnerEvents *pEvents, bool isThreadPool) :
                m_sender(sender), m_pEvents(pEvents), m_isThreadPool(isThreadPool)
            {
            }

        };

        

    }  // namespace TR

    // ----------------------------------------------------------

    // класс-обертка для MsgThread, позволяет создать в ее контексте объект любого типа
    class ThreadRunner
    {
        enum 
        { 
            CAsyncWaitTimeout = 45 * 1000, 
            CSleepTime = 50,
        };

        Platform::Thread::Priority m_priority;
        int m_timeout;
        int m_sleepTime;
        boost::shared_ptr<TR::Helper> m_helper;

        IThreadRunner& getThreadRunner()
        {
            ESS_ASSERT(m_helper.get() != 0);
            return  *(m_helper.get());
        }

        template<class TObject, class TObjectParams>
        TObject* AsyncCreate(TObjectParams &params)
        {
            typedef TR::Creator<TObject, TObjectParams> TCreator;

            boost::shared_ptr<TCreator> task( new TCreator(params, getThreadRunner()) );
            getThreadRunner().getThread().RunAsyncTask(task);

            // wait result
            SyncTimer t;

            while(task->Result() == 0)
            {
                if (task->WasException()) return 0;
                if (t.Get() > CAsyncWaitTimeout) ESS_ASSERT(0 && "Timeout");
                Platform::Thread::Sleep(CSleepTime);
            }

            return task->Result();
        }

        void DisableOtherThreadSafeDelete(MsgObject *pObject)
        {
            pObject->DisableOtherThreadSafeDelete();
        }

        void DisableOtherThreadSafeDelete(void *pObject)
        {
            // nothing
        }

        template<class TObject>
        void AsyncDelete(TObject* pObject)
        {
            typedef TR::Destructor<TObject> TDestructor;

            MsgThread &thread = getThreadRunner().getThread();

            // если поток остановлен, то безопасное удаление объекта невозможно 
            // (не сработает Async процедура)
            if (!thread.IsRunning())
            {
                DisableOtherThreadSafeDelete(pObject);
                delete pObject;
                return;
            }

            boost::shared_ptr<TDestructor> task( new TDestructor(pObject) );
            thread.RunAsyncTask(task);

            // wait result
            SyncTimer t;

            while(!task->Result())
            {
                if (t.Get() > CAsyncWaitTimeout) 
                    ESS_ASSERT(0 && "Timeout");
                Platform::Thread::Sleep(CSleepTime);
            }
        }


        bool WaitLoop(IThreadRunnerEvents *pNotify)
        {
            SyncTimer t;

            while(!m_helper->Completed())
            {
                if (m_helper->PollTimerReset()) t.Reset();

                if (m_timeout > 0 && t.Get() > dword(m_timeout))
                {
                    if (pNotify) pNotify->OnTimeout(getThreadRunner());
                    return false;
                }

                if (!m_helper->getThread().IsRunning()) 
                {
                    if (pNotify) pNotify->OnThreadUnexpectedStop(getThreadRunner());
                    return false;
                }

                FinalizeThreads();

                Platform::Thread::Sleep(m_sleepTime);
            }

            return true;
        }

        void FinalizeThreads();

    public:
        ThreadRunner(Platform::Thread::Priority priority = Platform::Thread::LowPriority,
            int timeout = 0) 
        {
            m_sleepTime = CSleepTime;

            setPriority(priority);
            setTimeout(timeout);
        }

        void setPriority(Platform::Thread::Priority value)
        {
            m_priority = value;
        }

        void setTimeout(int value)
        {
            m_timeout = value;
        }
        
        // конструктор TObject должен быть TObject(IThreadRunner &runner, TObjectParams &params)
        template<class TObject, class TObjectParams>
        bool Run(const std::string &name, TObjectParams &params, IThreadRunnerEvents *pNotify = 0, int sleepTime = CSleepTime)
        {
            using boost::scoped_ptr;

            m_sleepTime = sleepTime;

            // hooks must be upper MsgThread
            scoped_ptr<TR::HookThread> hookThread; 
            scoped_ptr<TR::HookThread> hookThreadPool; 

            MsgThread thread(name, m_priority, true);
            m_helper.reset( new TR::Helper(thread) );

            // hook setup
            {
                hookThread.reset( new TR::HookThread(getThreadRunner(), pNotify, false) );
                hookThreadPool.reset( new TR::HookThread(getThreadRunner(), pNotify, true) );

                thread.AddErrorHook( hookThread.get() );
                thread.getThreadPool().SetErrorHook( hookThreadPool.get() );
            }

            bool result = false;
            TObject *pObject = AsyncCreate<TObject>(params);
            if (pObject == 0) 
            {
                if (pNotify) pNotify->OnCreateObjectFail(getThreadRunner());
                return false;
            }

            try
            {
                result = WaitLoop(pNotify);
            }
            catch(...)
            {
                AsyncDelete(pObject);
                throw;
            }

            AsyncDelete(pObject);

            return result;
        }

        bool IsSame(const IThreadRunner &sender)
        {
            return &getThreadRunner() == &sender;
        }
    };
	
}  // namespace iCore

#endif

