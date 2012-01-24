#include "stdafx.h"
#include <VDK.h>
#include "TicksVisualTest.h"
#include "Utils/TimerTicks.h"

namespace BfDev
{
	
	void TicksVisualTest(Platform::dword timeoutMs, bool printToCout)
	{
	    Platform::word leds[6]    = {PF6, PF7, PF8, PF9, PF10, PF11};
	    Platform::word buttons[4] = {PF2, PF3, PF4, PF5};
	
		*pPORTFIO_DIR  |= (leds[0] | leds[1] | leds[2] | leds[3] | leds[4] | leds[5]);
		*pPORTFIO_INEN  = (buttons[0] | buttons[1] | buttons[2] | buttons[3]);
		*pPORTFIO_CLEAR = (leds[0] | leds[1] | leds[2] | leds[3] | leds[4] | leds[5]);
		
		if (printToCout)
		{
			std::cout << "Press any key to light LED for " << timeoutMs << " ms " << std::endl;			
		}
		
	    *pPORTFIO ^= leds[0];		
	    
	    while (true)
	    {
	    	if (*pPORTFIO & (buttons[0] | buttons[1] | buttons[2] | buttons[3]))
	        	break;
	    }
	
	    *pPORTFIO ^= leds[1];
	    
		Utils::TimerTicks t;
		t.Wait(timeoutMs);
		
		*pPORTFIO_CLEAR = leds[1];		
	}	
		
}  // namespace BfDef

