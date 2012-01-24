#include "stdafx.h"
#include "AdiTimerTest.h"


namespace BfDev
{
	AdiTimerTest::AdiTimerTest(int adiTimer, int counterTimer) : 
		m_adiTimer(adiTimer, this),
		m_timerCounter(counterTimer),
		m_stat(10)		
	{
		m_active = false;
		m_intervalMcsec = 0;
		m_lastTicks = 0;
	}
	
    // -------------------------------------------------

	void AdiTimerTest::Start(dword intervalMcsec)
	{
		ESS_ASSERT(!m_active);
		m_active = true;
		m_intervalMcsec = intervalMcsec;
		m_adiTimer.Start(intervalMcsec);
	}
	
    // -------------------------------------------------

	void AdiTimerTest::Stop()
	{
		ESS_ASSERT(m_active);			
		m_active = false;
		m_adiTimer.Stop();
	}		
	
    // -------------------------------------------------

	std::string AdiTimerTest::Result(dword testTimeSizeMcs) const
	{
		return m_stat.Result(m_intervalMcsec, testTimeSizeMcs);
	}
	
    // -------------------------------------------------

	void AdiTimerTest::Execute()
	{
		if(!m_active) return;
		dword currTick = m_timerCounter.GetCounter();
		m_stat.Update(currTick - m_lastTicks);
		m_lastTicks = currTick;
	}
	
    // -------------------------------------------------

	std::string AdiTimerTestStat::Result(dword intervalMcsec, dword testTimeSizeMcs) const
	{
		dword intervalCounter = BfTimerHelper::NsTimerToCounter(1000 * intervalMcsec);
		
		bool percent = false;
		dword lostInterrupt = testTimeSizeMcs;
		if(lostInterrupt != 0 && intervalMcsec != 0)
		{
			percent = true;
			lostInterrupt /= intervalMcsec;
		
			if(m_count > lostInterrupt)
				lostInterrupt = 0;
			else
				lostInterrupt = (100* (lostInterrupt - m_count))/lostInterrupt;  
		}
		
		float minIntervalMSec = BfTimerHelper::TimerCounterToNs(m_minInterval);
		minIntervalMSec /= 1000;
		float maxIntervalMSec = BfTimerHelper::TimerCounterToNs(m_maxInterval);
		maxIntervalMSec /= 1000;		

		float average = m_average;
		average /= m_count;
		average = BfTimerHelper::TimerCounterToNs(average);
		average /= 1000;
		
		std::ostringstream out;
		out << "Interval " << intervalMcsec << " Mcsec" << std::endl
        	<< "Interrupt count: " << m_count;
        if(percent) out << "; Lost: " << lostInterrupt << "%";
        out << std::endl
			<< "Average: " << std::fixed << std::setprecision(3) << average  << " Mcsec;" << std::endl        	
        	<< "Min: " << minIntervalMSec << " Mcsec;" << std::endl
        	<< "Max: " << maxIntervalMSec << " Mcsec;" 
        	<< " Max index: " << m_maxCount << std::endl;
        	
		return out.str();
	}	

};
