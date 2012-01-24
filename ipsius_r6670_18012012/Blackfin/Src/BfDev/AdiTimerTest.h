#ifndef _ADI_TIMER_TEST_
#define _ADI_TIMER_TEST_


#include "stdafx.h"
#include "Utils/VirtualInvoke.h" 
#include "AdiTimer.h" 
#include "BfTimerCounter.h"


namespace BfDev
{
	using Platform::dword;
	
	class AdiTimerTestStat : boost::noncopyable
	{
	public:
		AdiTimerTestStat(int ignoreFirst = 0) 
		{
			m_ignoreFirst = ignoreFirst;
			m_count = 0;
			m_average = 0;
			m_minInterval = 0xFFFFFFFF;
			m_maxInterval = 0;
			m_maxCount = 0;
		}
		void Update(dword val)
		{
			++m_count;
			if(m_count < m_ignoreFirst) return;
			
			m_average += val;
			if(m_minInterval > val) m_minInterval = val;
			if(m_maxInterval < val) 
			{
				m_maxInterval = val;
				m_maxCount = m_count;
			}
		}
		dword Count() const {	return m_count; }
		dword MinInterval() const {	return m_minInterval;	}
		dword MaxInterval() const {	return m_maxInterval;	}\
		std::string Result(dword intervalMcsec = 0, dword testTimeSizeMcs = 0) const;
	private:
		dword m_count;			
		int m_ignoreFirst;		
		Platform::ddword m_average;
		dword m_minInterval;
		dword m_maxInterval;
		dword m_maxCount;				
	};
	
		
	class AdiTimerTest : boost::noncopyable,
		Utils::IVirtualInvoke
	{
	public:
		AdiTimerTest(int adiTimer, int counterTimer);
		void Start(dword intervalMcsec);
		void Stop();
		std::string Result(dword testTimeSizeMcs = 0) const;		
	// Utils::IVirtualInvoke		
	private:		
		void Execute();
	private:		
		bool m_active;
		dword m_intervalMcsec;
		AdiTimer m_adiTimer;	
		BfTimerCounter m_timerCounter;
		dword m_lastTicks;
		AdiTimerTestStat m_stat;
	};		
};
		
#endif
		
