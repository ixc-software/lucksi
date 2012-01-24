#ifndef __BFLED__
#define __BFLED__

#include <cdefBF537.h>
#include <ccblkfn.h>


namespace DevIpTdm
{
	
	enum
    {
    	OFF = 0,
    	GREEN,
    	RED,
    	YELLOW
    };

	
    static void LedSet(int num, bool on)
    {
		*pPORTGIO_DIR |= 0x000c;
        int mask = (1 << (2 + num));

        if (on) *pPORTGIO_SET   = mask;                
           else *pPORTGIO_CLEAR	= mask;  	    	
    }		
    
    static void LedToggle(int num)
    {
        int mask = (1 << (2 + num));
        *pPORTGIO_TOGGLE = mask;  	  
    }		
    
    static void SetColorLed(char state)
    {    	
    	*pPORTGIO_DIR		|= 0x0030;		// LEDs    	
		*pPORTGIO_CLEAR	= 0x0030;                    	    	
		if (state == YELLOW) *pPORTGIO_SET= 0x0030;                    	      
		if (state == RED)	*pPORTGIO_SET= 0x0020;
		if (state == GREEN)	*pPORTGIO_SET= 0x0010;
    }

    static void ColorLedToggle()
    {
    	*pPORTGIO_DIR		|= 0x0030;		// LEDs    	
		*pPORTGIO_TOGGLE = 0x0030;                
    }

    
    
	
};

#endif
