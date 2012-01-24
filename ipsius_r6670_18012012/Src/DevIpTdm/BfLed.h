#ifndef __BFLED__
#define __BFLED__

/*
    Ёмул€ци€
*/

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
	public:
		

        static void Set(int num, bool on)
        {
        }

        static void SetColor(Platform::byte state)
        {
        }
							
	};
	
        
}  // namespace DevIpTdm

#endif
