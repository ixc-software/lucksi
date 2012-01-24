#ifndef __IDOMAINEXCEPTIONHOOK__
#define __IDOMAINEXCEPTIONHOOK__

#include "Utils/IBasicInterface.h"

namespace Domain
{
    
    class IDomainExceptionHook : Utils::IBasicInterface
    {
    public:
        // return true for suppress exception e
        virtual bool DomainExceptionSuppress(const std::exception &e) = 0;
    };
    
}  // namespace Domain

#endif
