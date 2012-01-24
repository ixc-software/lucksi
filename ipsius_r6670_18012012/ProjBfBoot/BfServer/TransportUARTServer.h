#ifndef __TRANSPORTUARTSERVER__
#define __TRANSPORTUARTSERVER__


#include "stdafx.h"
#include "ITransportServer.h"


using namespace Platform;


namespace BfServer
{

    // Реализация интерфейса транспортного уровня стороны сервера для UART
    class TransportUARTServer : public ITransportServer, public ExecutionUnit
    {
        ITransportServerEvents &m_observer;
        word m_comPort;


    public:
        TransportUARTServer(word comPort, Executer &executer, ITransportServerEvents &observer);
        ~TransportUARTServer();

        void SendData(boost::shared_ptr<Packet> data);

    // ExecutionUnit implementation
    private:
        void OnExecute();
    };

} // namespace BfServer

#endif