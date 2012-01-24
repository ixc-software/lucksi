#ifndef __TIMER_H__
#define __TIMER_H__

#include <sys\exception.h>
#include <cdefBF537.h>
#include <ccblkfn.h>
#include "Platform/Platform.h"


namespace Timer
{
    using namespace Platform;

    /*
        Timer0 class
        .....
    */
        
	class Timer0
	{
	public:
	
	    Timer0(dword sysFreq);	    
   	    ~Timer0();	    

        void TimersInt(void);
        void InitTimers(void);
        void InitTimerInterrupts(void);
        void Delay(const unsigned long ulMs);
        unsigned int SetTimeout(const unsigned long ulTicks);
        unsigned long ClearTimeout(const unsigned int nIndex);
        bool IsTimedout(const unsigned int nIndex);		
                
    private:    
        volatile unsigned long  g_ulTickCount;
        byte                    m_maxCntTimers;
        dword                   m_sysFreq;
        typedef struct
        {
    	    bool m_IsActive;
    	    unsigned long m_ulTimeoutCounter;
        }countdowntimer;
   	    countdowntimer m_sCountDownTimer[5];
    
	};
	
}; //namespace Timer


#endif

