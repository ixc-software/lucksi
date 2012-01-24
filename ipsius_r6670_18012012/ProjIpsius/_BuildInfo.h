

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
            return ADD_DEBUG("AutoBuildInfo: rev 5994; build Mon May 23 17:56:34 2011 by Skiv");   // FULL_INFO 
        }
        
        static const char* CmpProtoInfo()
        {
            return "iCmp ver 57 proto-crc 0x2a4a167a data-crc 0x9d241404";   // CMP_INFO
        }
        
        static int Revision()
        {
            return 5994;    // REVISION
        }
    };
    
    #undef DEBUG_INFO
    #undef ADD_DEBUG

#endif


