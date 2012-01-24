
#ifndef __UDPMONITOR__
#define __UDPMONITOR__


#include "Utils/IBasicInterface.h"
#include "iCore/MsgThread.h"
#include "Utils/ManagedList.h"

#include "UlvTypes.h"
#include "UlvError.h"
#include "UdpLocalPoint.h"
#include "ISocketDataTransformer.h"

namespace Ulv
{
    using Platform::dword;

    // Call-back interface to UdpLogger owner
    class IUdpMonitorEvents : 
        public Utils::IBasicInterface
    {
    public:
        virtual void NewData(const QString &data, const Utils::HostInf &src) = 0;
        virtual void ErrorOccur(boost::shared_ptr<Error> error) = 0;
    };

    // ---------------------------------------------------------------

    // Using for openning list of UDP sockets 
    class UdpMonitor : 
        public IUdpLocalPointEvents,
        boost::noncopyable
    {
        iCore::MsgThread &m_thread;
        IUdpMonitorEvents &m_owner;

        // for receiving data
        Utils::ManagedList<UdpLocalPoint> m_ports;
        
    // IUdpLocalPointEvents
    private:
        void NewData(const QString &data, const Utils::HostInf &src);
        void ErrorOccur(boost::shared_ptr<Error> error);

    public:
        UdpMonitor(iCore::MsgThread &thread, IUdpMonitorEvents &owner);

        void AddPort(int port, UdpPortDataType dataType);
        void Close();
    };

    
} // namespace Ulv

#endif
