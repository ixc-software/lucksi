#ifndef __BFTIMERCOUNTER__
#define __BFTIMERCOUNTER__

#include "Platform/Platform.h"
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
        ddword m_value;

        static int TimerCounterDiv(dword counter, int sysFreq, dword div)
        {
            // result = counter / (sysFreq / div)

            ddword val = counter;
            val *= div;
            val /= sysFreq;

            return val;
        }
                				
	public:
		
		BfTimerCounter(byte timerNumber)
		{
            Reset();
		}

        ~BfTimerCounter()
        {
        }
		
		void Reset()
		{
            m_value = GetSystemMicrosecTickCount();
		}
		
		dword GetCounter()
		{
			return GetSystemMicrosecTickCount() - m_value;
		}

        int CounterToMcs(dword counter)
        {
            return counter;
        }

        int CounterToNs(dword counter)
        {
            return counter * 1000;
        }

        /*
        int NsToCounter(dword ns)
        {
            return 0;
        }                

        void Sleep(dword interval)
        {
            ESS_HALT("");
        } */

        /*
        static int TimerCounterToMcs(dword counter, int sysFreq)
        {
            return counter;
        } */

        static int TimerCounterToMcs(dword counter, int sysFreq)
        {
            const int MHz = 1000 * 1000;
            return TimerCounterDiv(counter, sysFreq, MHz);
        }
		
        static int TimerCounterToMcs(dword counter)
        {
            return counter;
        }

        static int GetFreeTimer();
				
	};
	
}  // namespace BfDev


#endif

