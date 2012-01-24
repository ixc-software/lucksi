#include "stdafx.h"

#include "Platform/Platform.h"

#include "Semaphore.h"

// ---------------------------------

namespace iVDK
{

    void Semaphore::Post()
    {
        if (Platform::InsideIrqContext())
        {
            // VdkIsrPostSemaphore(m_id);
            VDK::C_ISR_PostSemaphore(m_id);
            return;
        }

        VDK::PostSemaphore(m_id);
    }

    bool Semaphore::Pend(dword timeout)
    {
        bool result;

        while(true)
        {
            result = VDK::PendSemaphore(m_id, timeout);
            if ((timeout == LockWaitForever) && !result) continue;
            break;
        }

        return result;
    }


}  // namespace iVDK
