#ifndef __PLATFORMWAITCONDITION__
#define __PLATFORMWAITCONDITION__

#include "Platform/PlatformMutex.h"

namespace Platform
{
    static const dword LockNoWait      = 0;
    static const dword LockWaitForever = 0xFFFFFFFF;
	
    class WaitCondition : QWaitCondition
	{
	public:

        bool wait(Mutex &mutex, dword timeout)
        {
            unsigned long t = timeout;
            if (t == LockWaitForever) t = ULONG_MAX;

            return QWaitCondition::wait(&mutex.getImpl(), timeout);
        }

        bool wait(Mutex &mutex)
        {
            return QWaitCondition::wait(&mutex.getImpl());
        }

        void wakeOne()
        {
            QWaitCondition::wakeOne();
        }
	};
	
}  // namespace Platform

#endif

