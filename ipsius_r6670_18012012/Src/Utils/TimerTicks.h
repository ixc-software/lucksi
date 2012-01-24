#ifndef __TIMERTICKS__
#define __TIMERTICKS__

#include "Utils/TimerBase.h"
#include "Platform/Platform.h"

namespace Utils
{
    using Platform::dword;
   
    // Таймер, основанный на тиках платформы
	class TimerTicks : public Utils::TimerBase
	{
		
	protected:

		dword GetTicks()  // override
		{
			return Ticks();
		}

	public:

		TimerTicks()
		{
			Init();
			Reset();
		}

        static dword Ticks()
        {
            return Platform::GetSystemTickCount();
        }

        void Sleep(dword interval)  // override
        {
            Platform::ThreadSleep(interval);
        }


	};
		
}

#endif

