
#ifndef __ISBPUSERTRANSPORTSERVER__
#define __ISBPUSERTRANSPORTSERVER__

// ISbpUserTransportServer.h

#include "Utils/IBasicInterface.h"
#include "ISbpTransport.h"

namespace SBProto
{
    // Call-back interface to transport server user
    class ISbpUserTransportServerEvents: public Utils::IBasicInterface
    {
    public:
        virtual void NewConnection(boost::shared_ptr<ISbpTransport> transport) = 0;
        virtual void TransportServerError(const std::string &err) = 0;
    };

    // ------------------------------------------------------------
    // Base interface for all SafeBiProto transport servers
    class ISbpUserTransportServer: public Utils::IBasicInterface
    {
    public:
        virtual void Start() = 0;
    };
    
} // namespace SBProto


#endif 
