#include "stdafx.h"

#include <ccblkfn.h>
#include <cdefBF537.h>

#include "leds.h"



namespace BfDev
{
    
    //-------------------------------------------------------------------

    Leds::Leds()
    {
        InitLeds();        
    }

    
    //-------------------------------------------------------------------

    void Leds::SetLed(byte state)
    {    	
    	*pPORTGIO_DIR		|= 0x0030;		// LEDs    	
		*pPORTGIO_CLEAR	= 0x0030;                    	    	
		if (state == YELLOW) *pPORTGIO_SET= 0x0030;                    	      
		if (state == RED)	*pPORTGIO_SET= 0x0020;
		if (state == GREEN)	*pPORTGIO_SET= 0x0010;
    }
    //-------------------------------------------------------------------

    void Leds::LedToggle()
    {
    	*pPORTGIO_DIR		|= 0x0030;		// LEDs    	
		*pPORTGIO_TOGGLE = 0x0030;                
    }

    //-------------------------------------------------------------------

    void Leds::InitLeds(void)
    {
    	int temp;	
	
    	*pPORTGIO_DIR		|= 0x0030;		// LEDs
		SetLed(OFF);
    }
} //namespace BfDev
