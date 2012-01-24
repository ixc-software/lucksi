#ifndef _MINI_BOOT_UART_BOOT_IMPL_H_
#define _MINI_BOOT_UART_BOOT_IMPL_H_

#include "NObjMiniFlasher.h"
#include "MfProcess.h"

namespace MiniFlasher
{
	using Platform::dword;

	class UartBootImpl : IMfProcessUser
	{
        void WaitCts(MfProcess &p);
        void Connect(MfProcess &p);
        void Write(MfProcess &p);
        
    // IMfProcessUser impl;
    private:
        void RunImpl(MfProcess &p); // can throw
        
    public:
        // Required parameters:
        // - uart profile
        // - file name
        UartBootImpl(MfProcessParams params);
	};
    
} // namespace MiniFlasher

#endif
