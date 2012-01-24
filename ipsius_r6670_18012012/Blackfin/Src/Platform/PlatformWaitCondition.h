#ifndef __PLATFORMWAITCONDITION__
#define __PLATFORMWAITCONDITION__

#include "Platform/PlatformMutex.h"
#include "iVDK/Semaphore.h"

namespace Platform
{
	
    static const dword LockNoWait      = iVDK::LockNoWait;
    static const dword LockWaitForever = iVDK::LockWaitForever;
	
	class WaitCondition
	{
        iVDK::Semaphore m_semaphore;

	public:

        WaitCondition()  {}
        ~WaitCondition() {}

        bool wait(Mutex &mutex, dword timeout = LockWaitForever);
        void wakeOne();
	};
	
}  // namespace Platform

#endif

