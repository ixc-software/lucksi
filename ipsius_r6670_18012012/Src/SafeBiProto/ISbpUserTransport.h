
#ifndef __ISBPUSERTRANSPORT__
#define __ISBPUSERTRANSPORT__

// ISbpUserTransport.h

#include "Utils/IBasicInterface.h"

namespace SBProto
{
    // -----------------------------------------------------------

    // Call-back interface from transport to user
    class ISbpTransportToUser: public Utils::IBasicInterface
    {
    public:
        virtual void TransportConnected() = 0;
        virtual void TransportDisconnected(const std::string &desc) = 0;
        virtual void TransportError(const std::string &err) = 0;
    };

    // -----------------------------------------------------------
    // Base interface for all transports using by users
    class ISbpUserToTransport : public Utils::IBasicInterface
    {
    public:
        virtual void Connect() = 0;
        virtual void Disconnect() = 0;
        virtual void BindUser(ISbpTransportToUser *pUser) = 0;
        virtual std::string Info() const = 0;
        virtual void Process() = 0;
    };

    
} // namespace SBProto

#endif
