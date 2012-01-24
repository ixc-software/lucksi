
#include "stdafx.h"
#include "Random.h"
#include "ErrorsSubsystem.h"

#define RND_CONST 0x08088405UL

#define HI_PART(x) ((word)( ( (x) >> 16) & 0xFFFF ))
#define LO_PART(x) ((word)( (x) & 0xFFFF))
#define COMPILE_DW(hi, lo)  ( (lo) | ( ( (dword)(hi) ) << 16 & 0xFFFF0000UL) )

namespace
{
    using Platform::word;
    using Platform::dword;
    using Platform::byte;
     

    // ------------------------------------------------------------------

    dword HiOfChainMult32(dword m1, dword m2)    
    {       
        word c;
        word w[4];
        dword uv;

        w[0] = 0;
        w[1] = 0;
        w[2] = 0;
        w[3] = 0;

        // 1 part
        uv = ((dword)LO_PART(m1)) * LO_PART(m2) + ((dword)w[0]);

        w[0] = LO_PART(uv);
        c = HI_PART(uv);

        // 2 part
        uv = ((dword)HI_PART(m1)) * LO_PART(m2) + ((dword)c) + ((dword)w[1]);

        w[1] = LO_PART(uv);
        w[2] = HI_PART(uv);

        // 3 part
        uv = ((dword)LO_PART(m1)) * HI_PART(m2) + ((dword)w[1]);

        w[1] = LO_PART(uv);
        c = HI_PART(uv);

        // 4 part
        uv = ((dword)HI_PART(m1)) * HI_PART(m2) + ((dword)c) + ((dword)w[2]);

        w[2] = LO_PART(uv);
        w[3] = HI_PART(uv);

        return COMPILE_DW(w[3], w[2]);
        //result->hi = COMPILE_DW(w[3], w[2]);
        //result->lo = COMPILE_DW(w[1], w[0]);
    }    

	// ------------------------------------------------------------------

    dword HiOfMult32(dword m1, dword m2)    
    {
        Platform::ddword ddw = (Platform::ddword)m1 * m2;
        return ddw >> 32;	
    }

};

namespace Utils
{
    using Platform::word;
    using Platform::byte;	
    using Platform::ddword;
	    
    // ------------------------------------------------------------------------------------

	dword Random::Next(dword range)
	{
        dword res;
        res = m_randSeed * RND_CONST;
        ++res;
        m_randSeed = res;

        if (m_useChainMult) return HiOfChainMult32(res, range);    
        return HiOfMult32(res, range);
	}    
    
	// ------------------------------------------------------------------

    Random::Random() : m_useChainMult(false)
    {
        setSeed(1);
    }

    void Random::Test() // static
    {
        const int CSeed = 0;
        const int CMaxIteration = 20000;

        Random normal(0);
        Random withChain(0, true);

        for (int i = 0; i < CMaxIteration; ++i)
        {
            TUT_ASSERT(normal.NextByte() == withChain.NextByte());
        }
    }


	std::string Random::RandomString(int len) 
	{
		static const char alphanum[] =
			"0123456789"
			"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
			"abcdefghijklmnopqrstuvwxyz";
		
		std::string res(" ", len);
		for (int i = 0; i < len; ++i) {
			res.at(i) = alphanum[NextByte() % (sizeof(alphanum) - 1)];
		}
		return res;
	}

	/*
	word IRandomAsm(dword range)
	{
	dword res;

	__asm
	{
	push eax;
	push edx;

	mov  eax, range;

	IMUL    EDX, rand_seed, RND_CONST;
	INC     EDX
	MOV     rand_seed, EDX
	MUL     EDX
	MOV     EAX,EDX

	mov  res, eax;

	pop  edx;
	pop  eax;

	}

	return (res);
	}
	*/
			
	
}  // namespace

#undef RND_CONST
#undef HI_PART
#undef LO_PART
#undef COMPILE_DW


