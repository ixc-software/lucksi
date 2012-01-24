
#ifndef __INETTCPTRANSPORTSCREATOR__
#define __INETTCPTRANSPORTSCREATOR__

// iNetTcpTransportsCreator.h
#include "Utils/SafeRef.h"
#include "ITestTransportsCreator.h"

#include "SafeBiProto/SbpTcpTransportFactory.h" 
#include "SafeBiProto/SbpTcpTransportServer.h"


namespace SBPTests
{
    using namespace SBProto;

    namespace iNetTcpTransportsCreatorDetail
    {
        const Utils::HostInf CServerHost = Utils::HostInf("127.0.0.1", 9856);
    }
        
    // -----------------------------------------------------------------

    // Create test transports based on iNet Tcp sockects 
    class iNetTcpTransportsCreator : public ITestTransportsCreator
    {
    // ITestTransportsCreator impl
    private:
        boost::shared_ptr<ISbpTransport> CreateTransport(ISbpTransportToUser &user, 
                                                         iCore::MsgThread &thread)
        {
			return SBProto::SbpTcpTransportFactory::CreateTransport(thread, 
                                            iNetTcpTransportsCreatorDetail::CServerHost, 
                                            &user);
        }
        
        boost::shared_ptr<ISbpUserTransportServer> CreateTransportServer(
                                                        ISbpUserTransportServerEvents &user, 
                                                        iCore::MsgThread &thread)
        {
            return boost::shared_ptr<SbpTcpTransportServer>(
                        new SbpTcpTransportServer(user, thread, 
                                                  iNetTcpTransportsCreatorDetail::CServerHost));
        }
    };
    
} // namespace SBPTests

#endif
