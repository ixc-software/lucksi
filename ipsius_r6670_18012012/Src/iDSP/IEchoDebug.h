#ifndef __IECHODEBUG__
#define __IECHODEBUG__

#include "Utils/IBasicInterface.h"

namespace iDSP
{
    
    class IEchoDebug : public Utils::IBasicInterface
    {
    public:
        
        virtual void OnAdoptEnter(int factor) = 0;
        virtual void OnAdoptLeave() = 0;
    };
    
    
}  // namespace Utils

#endif

