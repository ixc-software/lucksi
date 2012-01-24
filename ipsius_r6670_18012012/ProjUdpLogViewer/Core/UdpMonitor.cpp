
#include "stdafx.h"
#include "UdpMonitor.h"
#include "UdpDataTransformer.h"

namespace Ulv
{
    UdpMonitor::UdpMonitor(iCore::MsgThread &thread, IUdpMonitorEvents &owner) :
            m_thread(thread), m_owner(owner), m_ports(true)
    {
    }

    // ----------------------------------------------------------------------

    void UdpMonitor::NewData(const QString &data, const Utils::HostInf &src)
    {
        m_owner.NewData(data, src);
    }

    // ----------------------------------------------------------------------

    void UdpMonitor::ErrorOccur(boost::shared_ptr<Error> error)
    {        
        m_owner.ErrorOccur(error);
    }

    // ----------------------------------------------------------------------

    void UdpMonitor::AddPort(int port, UdpPortDataType dataType)
    {
        // Utils::HostInf host("127.0.0.1", port);
        Utils::HostInf host("0.0.0.0", port);
        m_ports.Add(new UdpLocalPoint(m_thread, *this, new UdpDataTransformer(dataType), host));
    }

    // ----------------------------------------------------------------------

    void UdpMonitor::Close()
    {
        // std::cout << "Ports count: " << m_ports.Size() << std::endl;
        m_ports.Clear();
    }

} // namespace Ulv

