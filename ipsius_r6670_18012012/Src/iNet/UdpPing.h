#ifndef _UDP_BROADCAST_H_
#define _UDP_BROADCAST_H_

#include "stdafx.h"
#include "Platform/PlatformTypes.h"
#include "Utils/SafeRef.h"
#include "Utils/TimerTicks.h"
#include "Utils/HostInf.h"

namespace iNet { class MsgUdpSocket; };
namespace Lw { class UdpSocket; };

namespace iNet
{

    class UdpPing : boost::noncopyable
    {
    public:
        UdpPing(Utils::SafeRef<Lw::UdpSocket>);
        UdpPing(Utils::SafeRef<MsgUdpSocket>);
        ~UdpPing();

        void Start(const Utils::HostInf &dstHost, const std::vector<Platform::byte> &data, int timeout)
        {
            m_dstHost = dstHost;
            m_timeout.Set(timeout, true);
            m_data = data;
        }
        void Stop()
        {
            m_dstHost.Clear();
            m_timeout.Stop();
        }
        void Process()
        {
            if (m_timeout.TimeOut()) SendImmediately();
        }
        void SendImmediately();
    private:
        Utils::HostInf m_dstHost;
        Utils::TimerTicks m_timeout;
        std::vector<Platform::byte> m_data;
        class Impl;
        boost::scoped_ptr<Impl> m_impl;
    };
};

#endif

