#include <ccblkfn.h>
#include "timer0.h"
#include "macros.h"


#define MAX_NUM_COUNTDOWN_TIMERS 5

static volatile unsigned long g_ulTickCount;


typedef struct
{
	bool m_IsActive;
	unsigned long m_ulTimeoutCounter;
}countdowntimer;

static countdowntimer sCountDownTimer[MAX_NUM_COUNTDOWN_TIMERS] = { {0,0},{0,0},{0,0},{0,0},{0,0} };
#define TIMEOUT_PERIOD SYS_CLK/1000


EX_INTERRUPT_HANDLER(Timer0_ISR)
{
	unsigned int n;
	// confirm interrupt handling
	*pTIMER_STATUS = 0x0001;
	ssync();

	g_ulTickCount++;

	// decrement each counter if it is non-zero
	for( n = 0;  n < MAX_NUM_COUNTDOWN_TIMERS; n++ )
	{
		if( 0 != sCountDownTimer[n].m_ulTimeoutCounter )
		{
			sCountDownTimer[n].m_ulTimeoutCounter--;
		}
	}
}


void Init_Timers(void)
{
	*pTIMER0_CONFIG		= 0x0019;
	*pTIMER0_PERIOD		= TIMEOUT_PERIOD;
	*pTIMER0_WIDTH		= TIMEOUT_PERIOD/2;	// width = period/2 = 50% duty cycle
	*pTIMER_ENABLE		= 0x0001;
	
	*pTIMER4_CONFIG		= 0x0009;
	*pTIMER4_PERIOD		= 256;
	*pTIMER4_WIDTH		= 2;	// width = period/2 = 50% duty cycle
	*pTIMER_ENABLE		= 0x0010;
	
}

void Init_Timer_Interrupts(void)
{
	// assign core IDs to interrupts
	*pSIC_IAR2 &= 0xFFFF0FFF;
	*pSIC_IAR2 |= 0x00004000;
	
	// assign ISRs to interrupt vectors
	register_handler(ik_ivg11, Timer0_ISR);		// Timer0 ISR -> IVG 11

	// enable Timer0 interrupt
	*pSIC_IMASK |= IRQ_TIMER0;
}

void Delay(const unsigned long ulMs)
{
	unsigned int uiTIMASK = cli();

	g_ulTickCount = 0;
	unsigned long ulEnd = ulMs;

	sti(uiTIMASK);

    while(g_ulTickCount < ulEnd )
	{
		asm("nop;");
	}
}

unsigned int SetTimeout(const unsigned long ulTicks)
{
	unsigned int uiTIMASK = cli();
	unsigned int n;

	// we don't care which countdown timer is used, so search for a free
	// timer structure
	for( n = 0;  n < MAX_NUM_COUNTDOWN_TIMERS; n++ )
	{
		if( false == sCountDownTimer[n].m_IsActive )
		{
			sCountDownTimer[n].m_IsActive = true;
			sCountDownTimer[n].m_ulTimeoutCounter = ulTicks;
			sti(uiTIMASK);
			return n;
		}
	}

	sti(uiTIMASK);
	return ((unsigned int)-1);
}

unsigned long ClearTimeout(const unsigned int nIndex)
{
	unsigned int uiTIMASK = cli();
	unsigned long ulTemp = (unsigned int)(-1);

	if( nIndex < MAX_NUM_COUNTDOWN_TIMERS )
	{
		// turn off the timer
		ulTemp = sCountDownTimer[nIndex].m_ulTimeoutCounter;
		sCountDownTimer[nIndex].m_ulTimeoutCounter = 0;
		sCountDownTimer[nIndex].m_IsActive = false;
	}

	sti(uiTIMASK);
	return (ulTemp);
}

bool IsTimedout(const unsigned int nIndex)
{
	unsigned int uiTIMASK = cli();
	if( nIndex < MAX_NUM_COUNTDOWN_TIMERS )
	{
		sti(uiTIMASK);
		return ( 0 == sCountDownTimer[nIndex].m_ulTimeoutCounter );
	}

	sti(uiTIMASK);
	return 0;// an invalid index should cause a hang wherever a timer is being used
}



