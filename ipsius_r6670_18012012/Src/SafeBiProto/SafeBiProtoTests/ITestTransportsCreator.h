
#ifndef __ITESTTRANSPORTSCREATOR__
#define __ITESTTRANSPORTSCREATOR__

// ITestTransportsCreator.h

#include "SafeBiProto/ISbpUserTransport.h"
#include "SafeBiProto/ISbpUserTransportServer.h"
#include "iCore/MsgThread.h"

namespace SBPTests
{
    using namespace SBProto;
    
    // Base interface for all test transports creators
    class ITestTransportsCreator: public Utils::IBasicInterface
    {
    public:
        virtual boost::shared_ptr<ISbpTransport> CreateTransport(ISbpTransportToUser &user, 
                                                                 iCore::MsgThread &thread) = 0;
        
        virtual boost::shared_ptr<ISbpUserTransportServer> CreateTransportServer(
                                                        ISbpUserTransportServerEvents &user, 
                                                        iCore::MsgThread &thread) = 0;
    };
    
} // namespace SBPTests

#endif
