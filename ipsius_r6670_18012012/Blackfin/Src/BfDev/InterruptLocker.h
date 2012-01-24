#ifndef INTERRUPTLOCKER_H
#define INTERRUPTLOCKER_H

#include <services/services.h>

namespace BfDev
{
    class InterruptLocker
    {
            void* m_pImask;
        public:
            InterruptLocker()
            {
                m_pImask = adi_int_EnterCriticalRegion(0);
            }

            ~InterruptLocker()
            {
                adi_int_ExitCriticalRegion(m_pImask);
            }
    };
    
    //-------------------------------------------------------
    
    class InterruptLockStrategy
    {
     public:
     
        class Mutex{};
        
        class Locker : public InterruptLocker
        {            
        public:
            Locker(Mutex&)
            {                
            }
        };
    };
    
    
} // namespace BfDev

#endif
