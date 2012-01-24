#ifndef __PLATFORMMUTEX__
#define __PLATFORMMUTEX__

#include "Utils/ErrorsSubsystem.h"
#include "iVDK/Mutex.h"
#include "iVDK/Semaphore.h"

namespace Platform
{
	
    // кросс-платформенная реализация мютекса
	class Mutex
	{
        iVDK::Semaphore m_semaphore;
        VDK::ThreadID m_threadID;

        static VDK::ThreadID BadThreadID()
        {
            return static_cast<VDK::ThreadID>(UINT_MAX);
        }

        bool Lock(bool justTry)
        {
            // dead lock check
            VDK::ThreadID current = VDK::GetThreadID();
            if (m_threadID == current)
            {
                ESS_HALT("Mutex deadlock!");
            }

            // lock
            bool res = m_semaphore.Pend(justTry ? 
                             iVDK::LockNoWait : 
                             iVDK::LockWaitForever);
            if (!res) return false;

            // set current thread
            m_threadID = current;

            return true;
        }

    public:

        Mutex() : m_threadID( BadThreadID() ) 
        {
        }        

        ~Mutex() {}

        void Lock()
        {
            Lock(false);
        }

        bool LockedByCurrentThread()
        {
            VDK::ThreadID current = VDK::GetThreadID();
            return (m_threadID == current);
        }

        void Unlock()
        {
            // check
            VDK::ThreadID current = VDK::GetThreadID();
            if (m_threadID != current)
            {
                ESS_HALT("Bad mutex unlock!");
            }

            // clear current thead
            m_threadID = BadThreadID();

            // unlock
            m_semaphore.Post();
        }
        
        bool TryLock()
        {
            return Lock(true);
        }

	};


}  // namespace Platform

#endif

