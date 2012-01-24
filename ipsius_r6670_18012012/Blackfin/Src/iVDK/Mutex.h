#ifndef __MUTEX__
#define __MUTEX__

#include "Platform/PlatformTypes.h"

namespace iVDK
{
	
	class Mutex : boost::noncopyable
	{
        Platform::dword m_id;

    public:

        Mutex();
        ~Mutex();

        void Lock();
        void Unlock();        
	};

    class MutexLocker : boost::noncopyable
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
	
}  // namespace iVDK

#endif

