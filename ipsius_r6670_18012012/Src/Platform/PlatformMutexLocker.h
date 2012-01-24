#ifndef __PLATFORMMUTEXLOCKER__
#define __PLATFORMMUTEXLOCKER__

#include "Platform/PlatformMutex.h"

namespace Platform
{

    // RAII solution for Mutex
    class MutexLocker
    {
        Mutex &m_mutex;

    public:

        MutexLocker(Mutex &mutex) : m_mutex(mutex)
        {
            m_mutex.Lock();
        }

        ~MutexLocker()
        {
            m_mutex.Unlock();
        }
    };

}  // namespace Platform

#endif

