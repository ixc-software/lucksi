#include "stdafx.h"
#include "Utils/ErrorsSubsystem.h"

#include "TcpServer.h"
#include "TcpServerImpl.h"

#include "TcpSocket.h"

namespace iNet
{
    // ----------------------------------------------------------------------------------------

    TcpServer::TcpServer(Utils::SafeRef<ITcpServerToUser> user) :
        m_impl(new TcpServer::Impl(user)){}

    // ----------------------------------------------------------------------------------------

    boost::shared_ptr<ITcpSocket> TcpServer::CreateTcpSocket(int socketDescriptor)
    {
        return TcpSocket::CreateTcpSocket(socketDescriptor);
    }

    // ----------------------------------------------------------------------------------------

    TcpServer::~TcpServer()
    {
        ESS_ASSERT(m_impl);

        m_impl->Delete();
        m_impl = 0;
    }

    // ----------------------------------------------------------------------------------------

    boost::shared_ptr<SocketError> TcpServer::Listen(const Utils::HostInf &host)
    {
        ESS_ASSERT(m_impl);

        return m_impl->Listen(host);
    }

    // ----------------------------------------------------------------------------------------

    void TcpServer::Close()
    {
        ESS_ASSERT(m_impl);

        m_impl->Close();
    }

    // ----------------------------------------------------------------------------------------

    Utils::HostInf TcpServer::LocalHostInf() const
    {
        ESS_ASSERT(m_impl);

        return m_impl->LocalHostInf();
    }
}
