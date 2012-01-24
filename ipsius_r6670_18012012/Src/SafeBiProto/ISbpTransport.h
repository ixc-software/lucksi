
#ifndef __ISBPTRANSPORT__
#define __ISBPTRANSPORT__

// ISbpTransport.h

#include "ISbpProtoTransport.h"
#include "ISbpUserTransport.h"

namespace SBProto
{
    // Common interface for all transports
    class ISbpTransport: 
        public ISbpProtoTransport, 
        public ISbpUserToTransport
    {
    };
    
} // namespace SBProto

#endif
