#ifndef __TRANSPORTTCPIPCLIENT__
#define __TRANSPORTTCPIPCLIENT__


#include "stdafx.h"
#include "ITransportClient.h"


using namespace Platform;


namespace BfBoot
{

    // Реализация интерфейса транспортного уровня стороны клиента для TCP/IP
   class TransportTCPIPClient : public ITransportClient
    {
        ITransportClientToProtocol &m_observer;
        std::string m_ipAddress;
        word m_port;


    public:
        TransportTCPIPClient(const std::string &ipAddress, word port, ITransportClientToProtocol &observer);
        ~TransportTCPIPClient();

        void Connect();
        void Disconnect();

        void SendData(boost::shared_ptr<Packet> data);
    };

} // namespace BfBoot

#endif