#include "stdafx.h"
#include "UdpPing.h"
#include "MsgUdpSocket.h"
namespace iNet
{
    class UdpPing::Impl : boost::noncopyable
    {
    public:
        Impl(Utils::SafeRef<MsgUdpSocket> &socket) : m_socket(socket)
        {}
        void Send(Utils::HostInf &dstHost, const std::vector<Platform::byte> &data)
        {
            ESS_ASSERT(!dstHost.Empty() && data.size() != 0);

            boost::shared_ptr<SocketData> msg(new SocketData(dstHost, 
                QByteArray(reinterpret_cast<const char*>(&data[0]), data.size())));

            m_socket->SendData(msg);
        }
    private:
        Utils::SafeRef<MsgUdpSocket> m_socket;
    };
    
    // -------------------------------------------------------------------

    UdpPing::UdpPing(Utils::SafeRef<MsgUdpSocket> socket) :
        m_impl(new Impl(socket))
    {}

    // -------------------------------------------------------------------

    UdpPing::~UdpPing()
    {}

    // -------------------------------------------------------------------

    void UdpPing::SendImmediately()
    {
        m_impl->Send(m_dstHost, m_data);
    }
};
