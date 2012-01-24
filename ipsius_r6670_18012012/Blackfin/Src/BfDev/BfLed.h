#ifndef __BFLED__
#define __BFLED__

#include <cdefBF537.h>
#include <ccblkfn.h>

#include "Utils/ErrorsSubsystem.h" 

namespace BfDev
{
	class BfLed
	{
		enum
		{
			CLedsCount = 6,
		};
		
		BfLed()
		{
            const int mask = 0x0FC0;

    		*pPORTFIO_DIR		|= mask;		// LEDs
    		*pPORTFIO_SET 		= mask;
    		*pPORTFIO_CLEAR		= mask;			
    		
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

            int mask = (1 << (6 + num));

            if (on) *pPORTFIO_SET   = mask;                
               else *pPORTFIO_CLEAR	= mask;  	    	
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
	
						
	};
	
};

#endif
