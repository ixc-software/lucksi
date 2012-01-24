#include "stdafx.h"
#include "VDK.h"
#include "Utils/ErrorsSubsystem.h"
#include "PlatformWaitCondition.h"

// --------------------------------------------

namespace Platform
{
	
	bool WaitCondition::wait(Mutex &mutex, dword timeout)
	{
    	if ( !mutex.LockedByCurrentThread() ) ESS_HALT("Mutex must be locked!");

        while (m_semaphore.getValue() > 0) m_semaphore.Pend();

        mutex.Unlock();
    	
    	bool res = m_semaphore.Pend(timeout);

        mutex.Lock();  // for RAII on user level

        return res;
	}
	
    void WaitCondition::wakeOne()
    {
        m_semaphore.Post();
    }
	
}  // namespace Platform
