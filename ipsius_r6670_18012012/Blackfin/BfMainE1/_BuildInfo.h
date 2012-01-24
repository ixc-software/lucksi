

#ifndef __BUILDINFO__
#define __BUILDINFO__

    // Don't edit this file!
    
    #ifdef NDEBUG 
        #define DEBUG_INFO ";RELEASE"
    #else
        #define DEBUG_INFO ";DEBUG"
    #endif
    
    #define ADD_DEBUG(msg) msg DEBUG_INFO
        

    struct AutoBuildInfo
    {
        static const char* FullInfo()
        {
            return ADD_DEBUG("AutoBuildInfo: rev 5370; build Wed Nov 24 18:12:54 2010 by Admin");   // FULL_INFO 
        }
        
        static const char* CmpProtoInfo()
        {
            return "iCmp ver 44 proto-crc 0x4fab082d data-crc 0x494a6b35";   // CMP_INFO
        }
        
        static int Revision()
        {
            return 5370;    // REVISION
        }
    };
    
    #undef DEBUG_INFO
    #undef ADD_DEBUG

#endif


