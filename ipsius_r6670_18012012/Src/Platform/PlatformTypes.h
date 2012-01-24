
#ifndef __PLATFORMTYPES__
#define __PLATFORMTYPES__

namespace Platform
{
    // basic type
    typedef boost::uint8_t  byte;     // 8  bits unsigned
    typedef boost::uint16_t word;     // 16 bits unsigned
    typedef boost::uint32_t dword;    // 32 bits unsigned
    typedef boost::uint64_t ddword;   // 64 bits unsigned
                                       
    typedef boost::int16_t int16;     // 16 bit signed
    typedef boost::int32_t int32;     // 32 bit signed
    typedef boost::int64_t int64;     // 64 bit signed

    BOOST_STATIC_ASSERT(sizeof(byte)    == 1);
    BOOST_STATIC_ASSERT(sizeof(word)    == 2);
    BOOST_STATIC_ASSERT(sizeof(dword)   == 4);
    BOOST_STATIC_ASSERT(sizeof(ddword)  == 8);
    
    BOOST_STATIC_ASSERT(sizeof(int32)  == 4);
    BOOST_STATIC_ASSERT(sizeof(int64)  == 8);

    typedef dword IntPtrValue;   // integer type for store pointer

    BOOST_STATIC_ASSERT(sizeof(IntPtrValue)  == sizeof(void*));
    
    enum
    {
        CMemoryAlign = 1,    // memory align
    };

}  // namespace Platform

#endif


