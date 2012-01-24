#ifndef _ADI_TIMER_H_
#define _ADI_TIMER_H_

#include "stdafx.h"
#include "Platform/Platform.h"
#include "Utils/ErrorsSubsystem.h" 
#include "Utils/VirtualInvoke.h" 
#include "BfTimerHelper.h"


namespace BfDev
{
	using Platform::dword;
	
	class AdiTimer : boost::noncopyable
	{
		static void Callback(void *AppHandle, dword  Event, void *pArg)
		{
			static_cast<Utils::IVirtualInvoke*>(AppHandle)->Execute();
		};
	public:
		ESS_TYPEDEF(InitializationFailed);
		
		AdiTimer(int number, Utils::IVirtualInvoke *observer = 0)		
		{
			Open(number);
			if(observer) InstallCallback(*observer);
		}
		~AdiTimer();
		
		void Start()
		{
			StartImpl(0xFFFFFFFF, 0xFFFFFFFF/2);
		}
	
		void Start(dword mcsec)	
		{
			StartImpl(BfTimerHelper::NsTimerToCounter(1000 * mcsec),  0xFFFFFFFF/2);	
		}
		void Stop();
		
		dword GetCounter() const;
		
        int CounterToMcs() const;
        int CounterToNs() const;
		void Sleep(dword interval) const;
		
		static void Test();		
		
	private:
		void Open(int number);
		void InstallCallback(Utils::IVirtualInvoke &observer);
		void StartImpl(dword period, dword width);	
	private:
		bool m_useInterrupt;
		bool m_isStarted;
		bool m_isActive;		
		
		dword m_startCounter;
		dword m_endCounter;		
		dword m_timerId;
     };
	
};  // namespace BfDev

#endif
