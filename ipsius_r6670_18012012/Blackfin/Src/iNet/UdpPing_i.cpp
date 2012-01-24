#include "stdafx.h"
#include "Lw/UdpSocket.h"
#include "UdpPing.h"


namespace iNet
{
    class UdpPing::Impl : boost::noncopyable
    {
    public:
        Impl(Utils::SafeRef<Lw::UdpSocket> socket) : m_socket(socket)
        {}
        void Send(const Utils::HostInf &dstHost, const std::vector<Platform::byte> &data)
        {
            ESS_ASSERT(!dstHost.Empty() && data.size() != 0);
            int size = data.size();
            m_socket->SendTo(dstHost, static_cast<const void*>(&data[0]), size);
        }
    private:
        Utils::SafeRef<Lw::UdpSocket> m_socket;
    };
    
    // -------------------------------------------------------------------

    UdpPing::UdpPing(Utils::SafeRef<Lw::UdpSocket> socket) :
        m_impl(new Impl(socket))
    {
    }

    // -------------------------------------------------------------------

    UdpPing::~UdpPing()
    {}

    // -------------------------------------------------------------------

    void UdpPing::SendImmediately()
    {
        m_impl->Send(m_dstHost, m_data);
    }
};
