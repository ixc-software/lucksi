#ifndef __BFKEYS__
#define __BFKEYS__

#include <cdefBF537.h>
#include <ccblkfn.h>

#include "Utils/ErrorsSubsystem.h" 

namespace DevIpTdm
{
    enum
    {
        KEY1 = 0,
    };
	
	
	class BfKeys
	{
		enum
		{
			CKeysCount = 1,
		};
		
		BfKeys()
		{
            const int mask = 0x0200;

    		*pPORTFIO_INEN		|= mask;		// Pushbuttons 
    		*pPORTFIO_SET 		= mask;   		
		}


        bool GetKeys(int num)
        {
            ESS_ASSERT(num < CKeysCount);

            int mask = (1 << (9 + num));

            return !(*pPORTFIO & mask);
        }

        static BfKeys& Instance()
        {
            static BfKeys keys;
            return keys;
        }
		
	public:
		

        static bool Get(int num)
        {
            return Instance().GetKeys(num);
        }
	
						
	};
	
};

#endif
