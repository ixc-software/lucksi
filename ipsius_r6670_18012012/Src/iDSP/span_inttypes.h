#if !defined(_INTTYPES_H_)
#define _INTTYPES_H_

#include "Platform/PlatformTypes.h"

#define inline __inline
#define __inline__ __inline

extern "C" 
{
    typedef Platform::byte uint8_t;
    typedef Platform::word 	 uint16_t;
    typedef Platform::dword	 uint32_t;
    typedef Platform::ddword uint64_t;

    typedef Platform::int16		int16_t;
    typedef Platform::int32		int32_t;
    typedef Platform::int64		int64_t;

    const int16_t INT16_MAX = 0x7fff;
    const int16_t INT16_MIN = (-INT16_MAX - 1);
//#define _MMX_H_ 
};

#endif
