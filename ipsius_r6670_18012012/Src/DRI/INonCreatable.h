#ifndef __INONCREATABLE__
#define __INONCREATABLE__

#include "Utils/IBasicInterface.h"

namespace DRI
{
    
    // For mark NamedObject, which can't created by DRI session
    class INonCreatable : public Utils::IBasicInterface
    {
    public:
        // nothing
    };    
    
    
}  // namespace DRI

#endif
