
#ifndef __ITELNETSESSIONMANAGTOOWNER__
#define __ITELNETSESSIONMANAGTOOWNER__

#include "Utils/IBasicInterface.h"


namespace DRI
{
    // Using inside Domain root object by NObjTelnetSessionManager 
    class ITelnetSessionManagerToOwner : public Utils::IBasicInterface
    {
    public:
        virtual void CloseSelf() = 0;
        virtual void EnableTrace(bool state) = 0;
        virtual QString GetStats() const = 0;
    };
    
} // namespace DRI

#endif
