#ifndef _PORT_H_
#define _PORT_H_

//--------------------------------------------------------------------------//
// Header files																//
//--------------------------------------------------------------------------//
#include <sys\exception.h>
#include <cdefBF537.h>
#include "macros.h"


extern byte led_timer;

//--------------------------------------------------------------------------//
// Prototypes																//
//--------------------------------------------------------------------------//
void Init_PIO(void);
void Init_PIO_Interrupts(void);
void Led_toggle(void);
void Set_Led(byte num, byte state);
void InitASYNC(void);


#endif

