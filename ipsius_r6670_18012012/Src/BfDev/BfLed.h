#ifndef __BFLED__
#define __BFLED__

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
		}

        void Clear()
        {
        }

        void SetLed(int num, bool on)
        {
            ESS_ASSERT(num < CLedsCount);
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
