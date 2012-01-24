#ifndef __BFLED__
#define __BFLED__

#include <cdefBF537.h>
#include <ccblkfn.h>

#include "Utils/ErrorsSubsystem.h" 
#include "Platform/Platform.h" 

namespace DevIpTdm
{

    enum
    {
    	OFF = 0,
    	GREEN,
    	RED,
    	YELLOW
    };


	class BfLed
	{
		enum
		{
			CLedsCount = 2,
		};
		
		BfLed()
		{
            const int mask = 0x000C;

    		*pPORTGIO_DIR		|= mask;		// LEDs   		
    		Clear();
		}

        void Clear()
        {
            for(int i = 0; i < CLedsCount; ++i)
            {
                Set(i, false);
            }
        }

        void SetLed(int num, bool on)
        {
            ESS_ASSERT(num < CLedsCount);

            int mask = (1 << (2 + num));

            if (on) *pPORTGIO_SET   = mask;                
               else *pPORTGIO_CLEAR	= mask;  	    	
        }

	void SetColorLed(Platform::byte state)
    	{    	
    		*pPORTGIO_DIR		|= 0x0030;		// LEDs    	
		*pPORTGIO_CLEAR	= 0x0030;                    	    	
		if (state == YELLOW) *pPORTGIO_SET= 0x0030;                    	      
		if (state == RED)	*pPORTGIO_SET= 0x0020;
		if (state == GREEN)	*pPORTGIO_SET= 0x0010;
    	}


        static BfLed& Instance()
        {
            static BfLed leds;
            return leds;
        }
		
	public:
		

        static void Set(int num, bool on)
        {
            Instance().SetLed(num, on);
        }

        static void SetColor(Platform::byte state)
        {
            Instance().SetColorLed(state);
        }

	
						
	};
	
	
} // namespace DevIpTdm

#endif
