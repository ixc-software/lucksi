#ifndef _TIMER_H_
#define _TIMER_H_

//--------------------------------------------------------------------------//
// Header files																//
//--------------------------------------------------------------------------//
#include <sys\exception.h>
#include <cdefBF537.h>

//--------------------------------------------------------------------------//
// Prototypes																//
//--------------------------------------------------------------------------//
void Init_Timers(void);
void Init_Timer_Interrupts(void);
void Delay(const unsigned long ulMs);
//unsigned long GetTickCount(void);
unsigned int SetTimeout(const unsigned long ulTicks);
unsigned long ClearTimeout(const unsigned int nIndex);
bool IsTimedout(const unsigned int nIndex);



#endif

