
#include "stdafx.h"

#include "TelnetServer.h"

namespace Telnet
{
    TelnetServer::TelnetServer(MsgThread &thread, 
        SafeRef<ITelnetServerEvents> owner,
        const Utils::HostInf &host)
        : MsgObject(thread), m_pOwner(owner),
        m_pSocket(shared_ptr<TelnetServerSocket>(new TelnetServerSocket(thread, this)))
    {
        bool allowZeroPort = true;
        int port = (!host.IsValid(allowZeroPort)) ? 23 : host.Port();
        Utils::HostInf tempHost(host.Address(), port);
		boost::shared_ptr<iNet::SocketError> error = m_pSocket->Listen(tempHost);
		if (error != 0)
		{
			m_pSocket->Close();
			m_pOwner->ServerSocketErrorInd(error);
		}
    }

    // implement ITcpServerSocketEvents
    void TelnetServer::NewConnection(iNet::SocketId id, 
                                     boost::shared_ptr<iNet::ITcpSocket> socket)
    {
        m_pOwner->ServerNewClientConnectInd(socket);
    }

    // -------------------------------------------

    Utils::HostInf TelnetServer::LocalHostInf() const
    {
        return m_pSocket->LocalHostInf(); 
    }

} // namesapce Telnet
