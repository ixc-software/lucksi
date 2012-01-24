#include "stdafx.h"
#include "timer0.h"
#include "TdmFramer.h"



   
namespace
{
   Timer::Timer0 *TimerSelf=0;
}


    EX_INTERRUPT_HANDLER(Timer0_ISR)
    {
        ESS_ASSERT(TimerSelf != 0);
        TimerSelf->TimersInt();
    }
    

namespace Timer
{
    
    
    Timer0::Timer0(dword sysFreq):
	    m_sysFreq(sysFreq)
	{	        
        ESS_ASSERT(TimerSelf == 0);
        TimerSelf = this;	        
	    m_maxCntTimers = 5;
	    for (int i=0; i<m_maxCntTimers; i++)
	    {
	        m_sCountDownTimer[i].m_IsActive = false;
            m_sCountDownTimer[i].m_ulTimeoutCounter = 0;	            
	    }            
	}

    
    Timer0::~Timer0()
	{	        
	    TimerSelf=0;
	}	        

    
    //------------------------------------------    
    
    void Timer0::TimersInt(void)
    {
    	unsigned int n;
    	// confirm interrupt handling
    	*pTIMER_STATUS = 0x0001;
    	ssync();

    	g_ulTickCount++;

    	// decrement each counter if it is non-zero
    	for( n = 0;  n < m_maxCntTimers; n++ )
    	{
    		if( 0 != m_sCountDownTimer[n].m_ulTimeoutCounter )
    		{
    			m_sCountDownTimer[n].m_ulTimeoutCounter--;
    		}
    	}        
    }    
    
    //------------------------------------------
    
    void Timer0::InitTimers(void)
    {
        int divider = static_cast<int>(m_sysFreq/1000);        
        
    	*pTIMER0_CONFIG		= 0x0019;
    	*pTIMER0_PERIOD		= divider;
    	*pTIMER0_WIDTH		= divider/2;	// width = period/2 = 50% duty cycle
    	*pTIMER_ENABLE		= 0x0001;
    }

    //------------------------------------------
        
    void Timer0::InitTimerInterrupts(void)
    {
    	
    	// assign core IDs to interrupts
    	*pSIC_IAR2 &= 0xFFFF0FFF;
    	*pSIC_IAR2 |= 0x00004000;
	
    	// assign ISRs to interrupt vectors
    	register_handler(ik_ivg11, Timer0_ISR);		// Timer0 ISR -> IVG 11

    	// enable Timer0 interrupt
    	*pSIC_IMASK |= IRQ_TIMER0;
    	
    }

    //------------------------------------------    
    
    void Timer0::Delay(const unsigned long ulMs)
    {
    	unsigned int uiTIMASK = cli();

    	g_ulTickCount = 0;
    	unsigned long ulEnd = (g_ulTickCount + ulMs);

    	sti(uiTIMASK);

        while( g_ulTickCount < ulEnd )
    	{
    		asm("nop;");
    	}
    }

    //------------------------------------------    
    
    unsigned int Timer0::SetTimeout(const unsigned long ulTicks)
    {
    	unsigned int uiTIMASK = cli();
    	unsigned int n;

    	// we don't care which countdown timer is used, so search for a free
    	// timer structure
    	for( n = 0;  n < m_maxCntTimers; n++ )
    	{
    		if( false == m_sCountDownTimer[n].m_IsActive )
    		{
    			m_sCountDownTimer[n].m_IsActive = true;
    			m_sCountDownTimer[n].m_ulTimeoutCounter = ulTicks;
    			sti(uiTIMASK);
    			return n;
    		}
    	}

    	sti(uiTIMASK);
    	return ((unsigned int)-1);
    }

    //------------------------------------------    
    
    unsigned long Timer0::ClearTimeout(const unsigned int nIndex)
    {
    	unsigned int uiTIMASK = cli();
    	unsigned long ulTemp = (unsigned int)(-1);

    	if( nIndex < m_maxCntTimers)
    	{
    		// turn off the timer
    		ulTemp = m_sCountDownTimer[nIndex].m_ulTimeoutCounter;
    		m_sCountDownTimer[nIndex].m_ulTimeoutCounter = 0;
    		m_sCountDownTimer[nIndex].m_IsActive = false;
    	}

    	sti(uiTIMASK);
    	return (ulTemp);
    }

    //------------------------------------------    
    
    bool Timer0::IsTimedout(const unsigned int nIndex)
    {
    	unsigned int uiTIMASK = cli();

    	if( nIndex < m_maxCntTimers )
    	{
    		sti(uiTIMASK);
     		return ( 0 == m_sCountDownTimer[nIndex].m_ulTimeoutCounter );
    	}

    	sti(uiTIMASK);
    	return 0;// an invalid index should cause a hang wherever a timer is being used
    }

} //namespace Timer


