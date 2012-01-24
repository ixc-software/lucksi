
#ifndef __TESTTCPTRANSPORTCREATOR__
#define __TESTTCPTRANSPORTCREATOR__

// TestTcpTransportCreator.h

#include "ITestTransportsCreator.h"
#include "TestTcpTransports.h"


namespace SBPTests
{
    namespace TestTcpTransportsCreatorDetail
    {
        const Utils::HostInf CServerHost = Utils::HostInf("127.0.0.1", 5857);
    }
    
    // -----------------------------------------------------------------

    // Create test transports based on TestUtils Tcp sockects 
    class TestTcpTransportsCreator : public ITestTransportsCreator
    {
    // ITestTransportsCreator impl
    private:
        boost::shared_ptr<ISbpTransport> CreateTransport(ISbpTransportToUser &user, 
                                                         iCore::MsgThread &thread)
        {
            return boost::shared_ptr<TestTcpTransport>(
                        new TestTcpTransport(thread, 
                                             TestTcpTransportsCreatorDetail::CServerHost, 
                                             &user));
        }
        
        boost::shared_ptr<ISbpUserTransportServer> CreateTransportServer(
                                                        ISbpUserTransportServerEvents &user, 
                                                        iCore::MsgThread &thread)
        {
            return boost::shared_ptr<TestTcpTransportServer>(
                        new TestTcpTransportServer(user, thread, 
                                                   TestTcpTransportsCreatorDetail::CServerHost));
        }
    };
    
    
} // namespace SBPTests

#endif
