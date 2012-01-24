#ifndef __BFSPI__
#define __BFSPI__

#include "Platform/Platform.h"
#include "BfDev/BfTimerCounter.h"
#include "Utils/TimerTicks.h"

namespace BfDev
{
    using namespace Platform;
	
    /*
        BfSpi class
        SPI bus driver
    */
                       
	class BfSpi : boost::noncopyable
	{		
			
	public:

		BfSpi(dword sysFreq, dword speedHz);
		~BfSpi();		
		byte Transfer(byte data);	
		dword GetSpeedHz() const;
		void SetSpeed(dword speedHz);
		
		// return -1 if error
		static int SpiBaud(dword sysFreq, dword speedHz)
		{									
			if (speedHz < 200*1000)     return -1;
		    if (speedHz > 30*1000*1000) return -1;
		    if (sysFreq * 2 < speedHz)  return -1;
		    
		    int baud = (sysFreq / 2) / speedHz;
		    if (baud == 0) return -1;
		    if (baud * 2 * speedHz != sysFreq) ++baud;
		    
		    return baud;
		}
			        		
    private:
    
    	bool CheckSpeed(dword speedHz);
		void Init();		
    	    	
		Utils::TimerTicks	m_timer;
        
    	dword m_sysFreq;    
		dword m_speedHz;

	};

} //namespace BfDev

#endif

