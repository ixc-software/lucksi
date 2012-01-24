#include "stdafx.h"

#include "Utils/ErrorsSubsystem.h"
#include "UdpSocket.h"
#include "UdpSocketImpl.h"

namespace iNet
{
    UdpSocket::UdpSocket(Utils::SafeRef<IUdpSocketToUser> iUdpSocketToUser, 
        const Utils::HostInf &host) :
        m_impl(new UdpSocket::Impl(iUdpSocketToUser, host)){}

    // ----------------------------------------------------------------------------------------

    UdpSocket::UdpSocket(SafeRef<IUdpSocketToUser> iUdpSocketToUser) :
        m_impl(new UdpSocket::Impl(iUdpSocketToUser)){}

    // ----------------------------------------------------------------------------------------

    bool UdpSocket::Bind(const Utils::HostInf &host)
    {
        return m_impl->Bind(host);

    }
        
    // ----------------------------------------------------------------------------------------

    UdpSocket::~UdpSocket()
    {
        ESS_ASSERT(m_impl);

        m_impl->Delete();

        m_impl = 0;
    }

    // ----------------------------------------------------------------------------------------

    void UdpSocket::SendData(boost::shared_ptr<SocketData> data)
    {
        ESS_ASSERT(m_impl);

        m_impl->SendData(data);
    }

    const Utils::HostInf &UdpSocket::LocalInf() const
    {
        ESS_ASSERT(m_impl);

        return m_impl->LocalHostInf();
    }

}
