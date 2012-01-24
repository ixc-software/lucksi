#ifndef __BFTIMERCOUNTER__
#define __BFTIMERCOUNTER__

#include <cdefBF537.h>
#include <ccblkfn.h>

#include "Platform/Platformtypes.h"
#include "Utils/ErrorsSubsystem.h" 
#include "SysProperties.h"

namespace BfDev
{
    using Platform::ddword;
	using Platform::dword;
	using Platform::word;		
	using Platform::byte;
	
	class BfTimerCounter
	{
        enum
        {
            CMaxTimers = 8,
        };

		byte m_number;
        bool m_forcedAlloc;  // do allocation without check what timer is free
		
		volatile word  *m_pConfig;
        volatile dword *m_pPeriod;
        volatile dword *m_pWidth;
        volatile dword *m_pCounter;

        dword m_sysFreq;
        				
		void Enable()
		{
			*pTIMER_ENABLE = (1 << m_number);
		}
		
		void Disable()
		{
			*pTIMER_DISABLE = (1 << m_number);	

            AbortPwmOut();		
		}
		
		#define SHIFT_WORD_REG(reg, num)  ( (volatile word*) ((dword)(reg) + (0x10 * num))  )
		#define SHIFT_DWORD_REG(reg, num) ( (volatile dword*)((dword)(reg) + (0x10 * num))  )
		
		void ShiftCheck()
		{
			ESS_ASSERT(pTIMER7_CONFIG   == SHIFT_WORD_REG(pTIMER0_CONFIG, 7));
			ESS_ASSERT(pTIMER7_PERIOD   == SHIFT_DWORD_REG(pTIMER0_PERIOD, 7));
			ESS_ASSERT(pTIMER7_WIDTH    == SHIFT_DWORD_REG(pTIMER0_WIDTH, 7));
        	ESS_ASSERT(pTIMER7_COUNTER  == SHIFT_DWORD_REG(pTIMER0_COUNTER, 7));			
		}

        static void AllocTimer(byte timerNumber);
        static void FreeTimer(byte timerNumber);
        static bool TimerEnabled(byte timerNumber);

        static int TimerCounterDiv(dword counter, int sysFreq, dword div)
        {
            // result = counter / (sysFreq / div)

            ddword val = counter;
            val *= div;
            val /= sysFreq;

            return val;
        }
        
        static int TimerNsDiv(dword nS, int sysFreq, dword div)
        {

            ddword val = nS;
            val *= sysFreq;
            val /= div;

            return val;
        }
        
        
        void ConfigTimer()
        {
			*m_pConfig = 0x0009;  			// PWM_OUT mode, count to period, use system clock
			*m_pPeriod = 0xFFFFFFFF;  		// count to max, use 0x0 - ?
			*m_pWidth  = 0xFFFFFFFF / 2;	// dummy - ?        	
        }

        void AbortPwmOut()
        {
            int bit = (m_number < 4) ? (m_number + 12) : (m_number + 24);

            *pTIMER_STATUS = (1 << bit);
        }

        void Init(byte timerNumber, dword sysFreq, bool forcedAlloc)
        {
            m_number = timerNumber;
            m_forcedAlloc = forcedAlloc;
            m_sysFreq = sysFreq;

            // verify
            ESS_ASSERT(m_number < CMaxTimers);
            ESS_ASSERT(m_sysFreq != 0);
            ShiftCheck(); // must be static

            if (!m_forcedAlloc) 
            {
                AllocTimer(m_number);
                if (TimerEnabled(m_number)) ESS_HALT("Reuse of active timer!");            
            }

            // set regs for timer
            m_pConfig   = SHIFT_WORD_REG(pTIMER0_CONFIG, m_number);
            m_pPeriod   = SHIFT_DWORD_REG(pTIMER0_PERIOD, m_number);
            m_pWidth    = SHIFT_DWORD_REG(pTIMER0_WIDTH, m_number);
            m_pCounter  = SHIFT_DWORD_REG(pTIMER0_COUNTER, m_number);

            // config
            ConfigTimer();

            Enable();
        }

				
	public:
		
		BfTimerCounter(byte timerNumber)
		{
            Init(timerNumber, SysProperties::Instance().getFrequencySys(), false);
		}

        BfTimerCounter(byte timerNumber, dword sysFreq, bool forcedAlloc)
        {
            Init(timerNumber, sysFreq, forcedAlloc); 
        }

        ~BfTimerCounter()
        {
            Disable();

            // mark timer as free
            // *m_pConfig = 0;

            FreeTimer(m_number);
        }
		
		void Reset()
		{
			Disable();			
            Enable();
		}
		
		dword GetCounter() const
		{
			return *m_pCounter;
		}

        int CounterToMcs(dword counter) const
        {
            return TimerCounterToMcs(counter, m_sysFreq);
        }

        int CounterToNs(dword counter) const
        {
            const int GHz = 1000 * 1000 * 1000;
            return TimerCounterDiv(counter, m_sysFreq, GHz);
        }

        int NsToCounter(dword ns) const
        {
            const int GHz = 1000 * 1000 * 1000;
            return TimerNsDiv(ns, m_sysFreq, GHz);
        }                

        void Sleep(dword interval) const
        {
            dword start = GetCounter();
            while(GetCounter() - start < interval);
        }

        static int TimerCounterToMcs(dword counter, int sysFreq)
        {
            const int MHz = 1000 * 1000;
            return TimerCounterDiv(counter, sysFreq, MHz);
        }
		
        static int TimerCounterToMcs(dword counter)
        {
            return TimerCounterToMcs( counter, SysProperties::Instance().getFrequencySys() );
        }

        static int GetFreeTimer();
	};

    void BfTimerCounterTest();
    void BfTimerReserveTimersMask(dword mask);
	
}  // namespace BfDev


#endif

