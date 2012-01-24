#ifndef __TRANSPORTUARTCLIENT__
#define __TRANSPORTUARTCLIENT__


#include "stdafx.h"
#include "ITransportClient.h"


using namespace Platform;


namespace BfBoot
{

    // Реализация интерфейса транспортного уровня стороны клиента для UART
    class TransportUARTClient : public ITransportClient
    {
        ITransportClientToProtocol &m_observer;
        word m_comPort;


    public:
        TransportUARTClient(word comPort, ITransportClientToProtocol &observer);
        ~TransportUARTClient();

        void Connect();
        void Disconnect();

        void SendData(boost::shared_ptr<Packet> data);
    };

} // namespace BfBoot

#endif