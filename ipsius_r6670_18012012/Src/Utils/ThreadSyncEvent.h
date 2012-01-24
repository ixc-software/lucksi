#ifndef __THREADSYNCEVENT__
#define __THREADSYNCEVENT__

#include "Platform/Platform.h"
#include "Utils/ErrorsSubsystem.h"
#include "AtomicTypes.h"

namespace Utils
{
	
    // Синхронное ожидание одного потока другим
    class ThreadSyncEvent
    {
        Platform::Mutex m_mutex;
        Platform::WaitCondition m_wait;
        Platform::Thread::ThreadID m_waitThreadID;
        AtomicBool m_readyDone;

    public:

        /* 
           Ждущий поток создает данный объект 
           При этом блокирует исполнение кода Ready() другим потоком, 
           пока сам не вызовет метод Wait()
        */
        ThreadSyncEvent() : m_readyDone(false)
        {
            m_waitThreadID = Platform::Thread::GetCurrentThreadID();
            m_mutex.Lock();
        }  

        ~ThreadSyncEvent()
        {
            ESS_ASSERT(m_readyDone.Get());
        }

        bool Wait(Platform::dword timeout = Platform::LockWaitForever) 
        {
            ESS_ASSERT(Platform::Thread::GetCurrentThreadID() == m_waitThreadID);
            bool result = m_wait.wait(m_mutex);
            m_mutex.Unlock();
            return result;
        }

        // Вызывается пробуждающим поток
        void Ready()
        {
            ESS_ASSERT( Platform::Thread::GetCurrentThreadID() != m_waitThreadID );

            m_mutex.Lock();
            m_wait.wakeOne();
            m_mutex.Unlock();

            m_readyDone.Set(true);
        }

    };
	
	class ThreadSyncEventUnlocker	: boost::noncopyable
	{
	public:
		ThreadSyncEventUnlocker(ThreadSyncEvent &threadSyncEvent) : m_threadSyncEvent(threadSyncEvent){}
		~ThreadSyncEventUnlocker() {	m_threadSyncEvent.Ready(); }
	private:
		ThreadSyncEvent &m_threadSyncEvent;
	};
	
}  // namespace Utils


#endif

