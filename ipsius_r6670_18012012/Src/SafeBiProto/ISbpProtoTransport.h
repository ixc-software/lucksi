
#ifndef __ISBPPROTOTRANSPORT__
#define __ISBPPROTOTRANSPORT__

// ISbpProtoTransport.h

#include "Utils/IBasicInterface.h"
#include "Utils/MemWriterStream.h"

namespace SBProto
{
    // Call-back interface to proto layer
    class ISbpTransportToProto : public Utils::IBasicInterface
    {
    public:
        virtual void DataReceived(const void *pData, size_t size) = 0;
    };
    
    // ------------------------------------------------

    // Common interface for all transports using by protocol
    class ISbpProtoTransport : public Utils::IBasicInterface
    {
        virtual void Send(const void *pData, size_t size) = 0;
    public:
        virtual size_t MaxSendSize() const = 0; // return 0 if no limit
        virtual void BindProto(ISbpTransportToProto *pProto) = 0;
        virtual void UnbindProto() = 0;
        void DataSend(const void *pData, size_t size)
        {
            if (MaxSendSize() != 0) ESS_ASSERT(MaxSendSize() >= size);
            Send(pData, size);
        }
        
    };

    
} // namespace SBProto

#endif
