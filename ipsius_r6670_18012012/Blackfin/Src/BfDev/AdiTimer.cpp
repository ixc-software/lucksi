#include "stdafx.h"
#include <services/services.h>
#include "InterruptLocker.h"
#include "AdiTimer.h" 
#include "BfTimerCounter.h"

namespace 
{
    using namespace BfDev;
    using namespace Platform;        

    void AdiTimerTestLoop()
    {

        AdiTimer timer(1);

        u32 lastTime = GetSystemTickCount();
        timer.Start();
        u32 lastTicks = timer.GetCounter();
		
        for(int i = 0; i < 4; i++)
        {
            Platform::ThreadSleep(20);

            dword currTime = Platform::GetSystemTickCount();
            dword currTicks = timer.GetCounter();

            std::cout << "Time " << (currTime - lastTime) << std::endl;
            std::cout << "Ticks " << BfTimerHelper::TimerCounterToMcs(currTicks - lastTicks) << " mcs" << std::endl;

            lastTime = currTime;
            lastTicks = currTicks;
        }
    }
	
    // -------------------------------------------------

    void AdiTimerReset(int timerIndx)
    {        
        const int CCount = 1024;

        AdiTimer t(timerIndx);
        t.Start();
    	while(t.GetCounter() < CCount);
    	t.Stop();
    	t.Start();    	
    	dword val = t.GetCounter();
    	ESS_ASSERT(val < CCount);    	
    }

	void SimpleTest()
	{
    	AdiTimerReset(1);
        AdiTimerReset(1);
    	
        AdiTimerTestLoop();
        AdiTimerTestLoop();

	};
	
    // -------------------------------------------------

	std::string AdiTmrResultToString(ADI_TMR_RESULT result)
	{
		switch(result)
		{
			case ADI_TMR_RESULT_SUCCESS:
				return "Generic success";
			case ADI_TMR_RESULT_FAILED:
				return "Generic failure";
			case ADI_TMR_RESULT_NOT_SUPPORTED:
				return "Not supported";
			case ADI_TMR_RESULT_BAD_TIMER_ID:
				return "Bad timer ID";
			case ADI_TMR_RESULT_BAD_TIMER_IDS:
				return "Bad timer IDs";
			case ADI_TMR_RESULT_BAD_TIMER_TYPE:
				return "Bad timer type";
			case ADI_TMR_RESULT_BAD_COMMAND:
				return "Bad command ID";
			case ADI_TMR_RESULT_INTERRUPT_MANAGER_ERROR:
				return "Eerror from interrupt manager";
			case ADI_TMR_RESULT_CALLBACK_ALREADY_INSTALLED:
				return "Callback already installed for timer";
		}
	
	
		ESS_HALT("Unknown code");
		return "Unknown code";
	}
	
    // -------------------------------------------------

	ADI_TMR_RESULT RunCommand(Platform::dword timerId, ADI_TMR_GP_CMD_VALUE_PAIR &command)	
	{
		return adi_tmr_GPControl(timerId, ADI_TMR_GP_CMD_PAIR, &command);
	}
	
};


namespace BfDev
{
	using Platform::dword;

	void AdiTimer::Open(int number)
	{
		m_isStarted = false;
		m_isActive = false;
		m_useInterrupt = false;
		m_startCounter = 0;
		m_endCounter = 0;		
		ESS_ASSERT(number >= 0 && number < ADI_TMR_GP_TIMER_COUNT );
		m_timerId = ADI_TMR_CREATE_GP_TIMER_ID(number);
		
		ADI_TMR_RESULT result = adi_tmr_Open(m_timerId);
		if (result != ADI_TMR_RESULT_SUCCESS) ESS_THROW_MSG(InitializationFailed, AdiTmrResultToString(result));
	}
	
    // -------------------------------------------------
    
	void AdiTimer::InstallCallback(Utils::IVirtualInvoke &observer)
	{
		m_useInterrupt = true;		
		ADI_TMR_RESULT result = adi_tmr_InstallCallback(m_timerId, TRUE, &observer, NULL, Callback);
		if (result != ADI_TMR_RESULT_SUCCESS) ESS_THROW_MSG(InitializationFailed, AdiTmrResultToString(result));
	}
	
	// ---------------------------------------------------------
	
	AdiTimer::~AdiTimer()
	{
		ADI_TMR_RESULT result = adi_tmr_Close(m_timerId);
		result = adi_tmr_RemoveCallback(m_timerId);
	}

	// ---------------------------------------------------------
	
	void AdiTimer::StartImpl(dword period, dword width)
	{
		ESS_ASSERT(!m_isStarted);
		{
			// задаем период
			ADI_TMR_GP_CMD_VALUE_PAIR configTable [] = {
				{ ADI_TMR_GP_CMD_SET_PERIOD,				(void *)period	}, 
				{ ADI_TMR_GP_CMD_SET_WIDTH,					(void *)width	}, 
				{ ADI_TMR_GP_CMD_SET_TIMER_MODE,			(void *)0x01	}, // PWM
				{ ADI_TMR_GP_CMD_SET_COUNT_METHOD,			(void *)TRUE	}, // count to end of period 
				{ ADI_TMR_GP_CMD_SET_INTERRUPT_ENABLE,		(void *)m_useInterrupt },
				{ ADI_TMR_GP_CMD_ENABLE_TIMER,	(void *)TRUE}, 				
				{ ADI_TMR_GP_CMD_END,		NULL			} 
			};
			ADI_TMR_RESULT result = adi_tmr_GPControl(m_timerId, ADI_TMR_GP_CMD_TABLE, configTable );
			if (result != ADI_TMR_RESULT_SUCCESS) ESS_THROW_MSG(InitializationFailed, AdiTmrResultToString(result));
		}
		m_isStarted = true;
		m_isActive = true;	
//		m_startCounter = GetCounter();
	}
	
	// ---------------------------------------------------------
	
	void AdiTimer::Stop()
	{
		ESS_ASSERT(m_isStarted);
		
		m_isStarted = false;
		m_isActive = false;
	
		ADI_TMR_RESULT result = adi_tmr_Reset(m_timerId);
		if(result != ADI_TMR_RESULT_SUCCESS) ESS_THROW_MSG(InitializationFailed, AdiTmrResultToString(result));		

		m_startCounter = 0;
		m_endCounter = 0;
	}

	// ---------------------------------------------------------
	
	dword AdiTimer::GetCounter() const
	{
		dword counter = 0;
		ADI_TMR_GP_CMD_VALUE_PAIR command = {ADI_TMR_GP_CMD_GET_COUNTER, (void*)&counter};
		RunCommand(m_timerId, command);
		return counter;		
	}

	// ---------------------------------------------------------
	
    int AdiTimer::CounterToMcs() const
    {
    		
		return BfTimerHelper::TimerCounterToMcs(GetCounter());
    }

	// ---------------------------------------------------------
	
    int AdiTimer::CounterToNs() const
    {
		return BfTimerHelper::TimerCounterToNs(GetCounter());
    }

    // ---------------------------------------------------------
	
	void AdiTimer::Sleep(dword interval) const
	{
		ESS_ASSERT(!m_useInterrupt);
		
        dword start = GetCounter();
        while(GetCounter() - start < interval);
	}
	
    // ---------------------------------------------------------

	void AdiTimer::Test() // static 
	{
		AdiTimerReset(1);
		AdiTimerReset(2);		
		AdiTimerTestLoop();
		AdiTimerTestLoop();		
	}
};  // namespace BfDev

