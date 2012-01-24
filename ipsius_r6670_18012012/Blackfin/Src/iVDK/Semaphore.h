#ifndef __VDK_SEMAPHORE__
#define __VDK_SEMAPHORE__

#include "VDK.h"
#include "Platform/PlatformTypes.h"
#include "Utils/ErrorsSubsystem.h"
#include "iVDK/VdkCheckInit.h"

namespace iVDK
{
    using Platform::dword;

    static const dword LockNoWait 		= VDK::kDoNotWait;
    static const dword LockWaitForever  = INT_MAX - 1;

    class Semaphore : boost::noncopyable
    {
        VDK::SemaphoreID m_id;
       
        void Create(dword initial, dword max)
        {
			VdkCheckInit();
        	
            m_id = VDK::CreateSemaphore(initial, max, 0, 0); 
            ESS_ASSERT(m_id != UINT_MAX);
        }

    public:

        Semaphore()
        {
            Create(1, 1);
        }

        Semaphore(dword initial, dword max)
        {
            Create(initial, max);
        }

        ~Semaphore()
        {
            VDK::DestroySemaphore(m_id);
        }

        dword getValue()
        {
            dword val = VDK::GetSemaphoreValue(m_id);
            ESS_ASSERT(val != UINT_MAX);

            return val;
        }

        
        void Post();  // inc      
        bool Pend(dword timeout = LockWaitForever);  // dec

    };



}  // namespace iVDK

#endif


