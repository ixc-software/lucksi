#ifndef _BFTIMER_HELPER_H_
#define _BFTIMER_HELPER_H_

#include <cdefBF537.h>
#include <ccblkfn.h>

#include "Platform/Platformtypes.h"
#include "SysProperties.h"

namespace BfDev
{
    using Platform::ddword;
	using Platform::dword;
	using Platform::word;		
	using Platform::byte;
	
	class BfTimerHelper
	{
        static dword TimerCounterDiv(dword counter, dword div)
        {
			return TimerCounterDiv(counter, SysProperties::Instance().getFrequencySys(), div);
        }
        
        static dword TimerCounterDiv(dword counter, int sysFreq, dword div)
        {
            // result = counter / (sysFreq / div)

            ddword val = counter;
            val *= div;
            val /= sysFreq;

            return val;
        }
        
        static dword TimerNsDiv(dword nS, dword div)
        {
			return TimerNsDiv(nS, SysProperties::Instance().getFrequencySys(), div);        	
        }
                
        static dword TimerNsDiv(dword nS, int sysFreq, dword div)
        {

            ddword val = nS;
            val *= sysFreq;
            val /= div;

            return val;
        }
        
	public:
		
        static dword TimerCounterToMcs(dword counter)
        {
            const dword MHz = 1000 * 1000;        	
            return TimerCounterDiv( counter, MHz );
        }

        static dword TimerCounterToNs(dword counter)
        {
            const dword GHz = 1000 * 1000 * 1000;
            return TimerCounterDiv(counter, GHz);
        }

        static dword NsTimerToCounter(dword ns)
        {
            const dword GHz = 1000 * 1000 * 1000;
            return TimerNsDiv(ns, GHz);
        }                
        
        static int GetFreeTimer();
	};
	
}  // namespace BfDev


#endif

