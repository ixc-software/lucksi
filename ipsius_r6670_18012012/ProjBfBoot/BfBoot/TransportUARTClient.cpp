#include "stdafx.h"
#include "TransportUARTClient.h"


using namespace Platform;


namespace BfBoot
{


    //-------------------------------------------------------------------------
    // TransportUARTClient implementation
    //-------------------------------------------------------------------------


    TransportUARTClient::TransportUARTClient(word comPort, ITransportClientToProtocol &observer) :
        ITransportClient(),
        m_observer(observer),
        m_comPort(comPort)
    {
    }


    //-------------------------------------------------------------------------


    TransportUARTClient::~TransportUARTClient()
    {
        // todo: Disconnect without event generation (if needed)
    }


    //-------------------------------------------------------------------------


    void TransportUARTClient::Connect()
    {
        // todo:
    }


    //-------------------------------------------------------------------------


    void TransportUARTClient::Disconnect()
    {
        // todo:
    }


    //-------------------------------------------------------------------------


    void TransportUARTClient::SendData(boost::shared_ptr<Packet> data)
    {
        // todo:
    }


} // namespace BfBoot