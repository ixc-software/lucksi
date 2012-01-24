
#ifndef __TIMERBASE__

#define __TIMERBASE__

#include "Platform/PlatformTypes.h"
#include "Utils/IBasicInterface.h"

namespace Utils
{
    using Platform::dword;
	
	class TimerBase : IBasicInterface
	{
		dword	m_startVal;
		dword	m_timeInterval;
		bool	m_autoresetMode;

		virtual dword GetTicks() = 0;

    protected:

        void Init();

	public:

        TimerBase();

		dword StartVal() const;
		void Reset();
		dword Get();

		void Set(dword interval, bool autoreset = true);
		bool TimeOut();
		void Stop();

		virtual void Sleep(dword interval);
        void Wait(dword interval);

	};
		
}


#endif



