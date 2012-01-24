
#ifndef _SBP_TCP_TRANSPORT_FACTORY_
#define _SBP_TCP_TRANSPORT_FACTORY_

// SbpTcpTransport.h

#include "Utils/HostInf.h"
#include "SafeBiProto/ISbpTransport.h"

namespace iCore { class MsgThread; };
namespace iNet { class ITcpSocket; };
namespace Lw { class TcpSocket; };

namespace SBProto
{
    // -----------------------------------------------------------

    // Base class for Tcp transports
    class SbpTcpTransportFactory
    {
    public:
        // client-side constructor: create own socket
		static boost::shared_ptr<ISbpTransport> CreateTransport(iCore::MsgThread &thread, 
			const Utils::HostInf &host,
			ISbpTransportToUser *pUser = 0, size_t maxSendSize = 0);


        // server-side constructor: using given socket
        static boost::shared_ptr<ISbpTransport> CreateTransport(boost::shared_ptr<iNet::ITcpSocket> socket, size_t maxSendSize = 0);

        static boost::shared_ptr<ISbpTransport> CreateTransport(Lw::TcpSocket *socket, size_t maxSendSize = 0);
    };
   
} // namespace SBProto

#endif







