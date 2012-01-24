#include "Platform/Platform.h"

// -----------------------------------------------

extern "C"
{
    
    void PlatfromExitWithMessage(char *pMsg)
    {
        Platform::ExitWithMessage(pMsg);
    }
    
}



