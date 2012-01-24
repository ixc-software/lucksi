#ifndef __PLATFORMBLACKFINMINI__
#define __PLATFORMBLACKFINMINI__

#include "Src/Platform/PlatformExitHandle.h"

namespace Platform
{
    
    #define MINI_ESS_ASSERT(_Expression) (void)( (!!(_Expression)) || (Platform::HandleAssert(#_Expression, __FILE__, __LINE__))) 
        
    // NB: it's don't work with std::string like original version
    #define MINI_ESS_HALT(textMsg)       (Platform::HandleAssert(textMsg, __FILE__, __LINE__))

    void ExitWithMessageExt(const char *pMsg, bool userContext);            
    void ExitWithMessage(const char *pMsg);
    
    bool InsideIrqContext();

    bool HandleAssert(const char *pMsg, const char *pFile, int line);
    
}  // namespace Platform


#endif

