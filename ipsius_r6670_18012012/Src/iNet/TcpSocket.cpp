#include "stdafx.h"
#include "Utils/ErrorsSubsystem.h"
#include "TcpSocket.h"
#include "TcpSocketImpl.h"


namespace iNet
{
    // ----------------------------------------------------------------------------------------

    TcpSocket::TcpSocket(Utils::SafeRef<ITcpSocketToUser> iTcpSocketToUser) :
        m_impl(new TcpSocket::Impl(iTcpSocketToUser))  {}

    // ----------------------------------------------------------------------------------------

    TcpSocket::TcpSocket() : m_impl(new TcpSocket::Impl) {}

    // ----------------------------------------------------------------------------------------

    TcpSocket::~TcpSocket()
    {
        ESS_ASSERT(m_impl);

        m_impl->Delete();
        m_impl = 0;
    }

    // ----------------------------------------------------------------------------------------

    bool TcpSocket::IsEqual(SocketId id) const
    {
        return id == ID();
    }
    
    // ----------------------------------------------------------------------------------------

    void TcpSocket::LinkUserToSocket(Utils::SafeRef<ITcpSocketToUser> iTcpSocketToUser)
    {
        ESS_ASSERT(m_impl);

        m_impl->LinkUserToSocket(iTcpSocketToUser);
    }

    // ----------------------------------------------------------------------------------------

    boost::shared_ptr<ITcpSocket> TcpSocket::CreateTcpSocket(int socketDescriptor)
    {
        boost::shared_ptr<TcpSocket> tcpSocket(new TcpSocket);

        if(!tcpSocket->InitSocket(socketDescriptor)) tcpSocket.reset();

        return tcpSocket;
    }

    // ----------------------------------------------------------------------------------------

    bool TcpSocket::InitSocket(int socketDescriptor)
    {
        ESS_ASSERT(m_impl);

        return m_impl->InitSocket(socketDescriptor);
    }

    // ----------------------------------------------------------------------------------------

    void TcpSocket::ConnectToHost(const Utils::HostInf &host)
    {
        ESS_ASSERT(m_impl);

        m_impl->ConnectToHost(host);
    }

    // ----------------------------------------------------------------------------------------

    void TcpSocket::SendData( const QByteArray &data)
    {
        ESS_ASSERT(m_impl);

        m_impl->SendData(data);
    }

    // ----------------------------------------------------------------------------------------

    void TcpSocket::DisconnectFromHost()
    {
        ESS_ASSERT(m_impl);

        m_impl->DisconnectFromHost();
    }

    // ----------------------------------------------------------------------------------------

    const Utils::HostInf &TcpSocket::LocalHostInf() const
    {
        ESS_ASSERT(m_impl);

        return m_impl->LocalHostInf();
    }

    // ----------------------------------------------------------------------------------------

    const Utils::HostInf &TcpSocket::PeerHostInf() const
    {
        ESS_ASSERT(m_impl);

        return m_impl->PeerHostInf();
    }

    // ----------------------------------------------------------------------------------------

    QString TcpSocket::getPeerName() const
    {
        ESS_ASSERT(m_impl);

        return m_impl->getPeerName();
    }
}
