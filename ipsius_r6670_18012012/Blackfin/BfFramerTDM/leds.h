#ifndef __LEDS__
#define __LEDS__

#include "Platform/Platform.h"

namespace BfDev
{
    using namespace Platform;

    /*
        Leds class
        .....
    */
    
    enum
    {
    	OFF = 0,
    	GREEN,
    	RED,
    	YELLOW
    };
        
	class Leds
	{
	public:

	    Leds();
        static void LedToggle();
        static void SetLed(byte state);	
        
    private:
    
        void InitLeds();    
    

	};

} //namespace BfDev

#endif

