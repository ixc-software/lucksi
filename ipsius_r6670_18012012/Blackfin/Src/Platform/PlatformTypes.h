
#ifndef __PLATFORMTYPES__
#define __PLATFORMTYPES__

#include "boost/static_assert.hpp"

namespace Platform
{
    // unsigned types
    typedef unsigned char			byte;     // 8  bits unsigned
    typedef unsigned short 			word;     // 16 bits unsigned
    typedef unsigned long  			dword;    // 32 bits unsigned
    typedef unsigned long long   	ddword;   // 64 bits unsigned

	// signed types  
    typedef short 					int16;     // 16 bit signed
    typedef int   					int32;     // 32 bit signed
    typedef long long 				int64;     // 64 bit signed


    BOOST_STATIC_ASSERT(sizeof(byte)    == 1);
    BOOST_STATIC_ASSERT(sizeof(word)    == 2);
    BOOST_STATIC_ASSERT(sizeof(dword)   == 4);
    BOOST_STATIC_ASSERT(sizeof(ddword)  == 8);
    
    BOOST_STATIC_ASSERT(sizeof(int16)    == 2);
    BOOST_STATIC_ASSERT(sizeof(int32)    == 4);
    BOOST_STATIC_ASSERT(sizeof(int64)    == 8);
        
    typedef dword IntPtrValue;   // integer type for store pointer

    BOOST_STATIC_ASSERT(sizeof(IntPtrValue)  == sizeof(void*));

    enum
    {
        CMemoryAlign = 4,    // memory align
    };

   
}  // namespace Platform

#endif


