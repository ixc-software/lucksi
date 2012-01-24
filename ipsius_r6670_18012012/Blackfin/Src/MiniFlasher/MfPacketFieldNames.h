
#ifndef __MFPACKETFIELDNAMES__
#define __MFPACKETFIELDNAMES__

// MfPacketFieldNames.h

namespace MiniFlasher
{
    /* Register packet fields names here */

    namespace MfPacketFieldNames
    {
        #define CONST_FIELD_NAME(m) const char* const m = #m;
            
        CONST_FIELD_NAME(FCode);
        CONST_FIELD_NAME(FVersion);
        CONST_FIELD_NAME(FSize);
        CONST_FIELD_NAME(FCRC32);
        CONST_FIELD_NAME(FErrors);
        CONST_FIELD_NAME(FData);
        CONST_FIELD_NAME(FPercent);

        // ...

        #undef CONST_FIELD_NAME
            
    } // namespace MfPacketFieldNames
    
} // namespace MiniFlasher

#endif
