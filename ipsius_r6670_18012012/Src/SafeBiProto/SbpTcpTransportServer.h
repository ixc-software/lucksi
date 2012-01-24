
#ifndef __SBPTCPTRANSPORTSERVER__
#define __SBPTCPTRANSPORTSERVER__

// SbpTcpTransportServer.h

#include "ISbpUserTransportServer.h"
#include "SbpTcpTransportFactory.h"

#include "Utils/SafeRef.h"
#include "iNet/SocketError.h"
#include "iNet/MsgTcpServer.h"


namespace SBProto
{

    class SbpTcpTransportServer:
        public virtual Utils::SafeRefServer,
        public iNet::ITcpServerToUser,
        public ISbpUserTransportServer,
        public boost::noncopyable
    {
        ISbpUserTransportServerEvents &m_owner;
        iNet::MsgTcpServer m_server;
        Utils::HostInf m_host;
        
    // ITcpServerToUser
    private:
        void NewConnection(iNet::SocketId id, boost::shared_ptr<iNet::ITcpSocket> socket)
        {
			m_owner.NewConnection(SbpTcpTransportFactory::CreateTransport(socket));
        }

    // ISbpUserTransportServer impl
    private:
        void Start()
        {
			boost::shared_ptr<iNet::SocketError> serverError = 
				m_server.Listen(m_host);
			if (serverError != 0)
			{
				m_owner.TransportServerError(serverError->getErrorString().toStdString());
			}
        }

    public:
        SbpTcpTransportServer(ISbpUserTransportServerEvents &owner, iCore::MsgThread &thread,
                              const Utils::HostInf &host)
        : m_owner(owner), m_server(thread, this), m_host(host)
        {
        }

        
    };

} // namespace SBProto


#endif 
