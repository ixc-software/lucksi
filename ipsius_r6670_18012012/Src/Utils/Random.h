
#ifndef __UTILS_RANDOM__

#define __UTILS_RANDOM__

#include "Platform/PlatformTypes.h"

namespace Utils
{
    using Platform::dword;
    using Platform::byte;

	class Random
	{
		dword    m_randSeed;
        /* const */ bool m_useChainMult;

	public:
        Random();

        Random(int seed, bool useChainMult = false) 
            : m_useChainMult(useChainMult)
	    {
		    setSeed(seed);
	    }

	    void setSeed(int seed)
	    {
		    m_randSeed = seed;
	    }

	    dword Next(dword range = 0xFFFFFFFF);
        

        byte NextByte()
        {
            return Next(0x100);
        }

		dword NextInRange(dword begin, dword end)
		{
			return begin + Next(end - begin + 1);
		}

		std::string RandomString(int len);

        static void Test();
		
	};

}

#endif
