#include "stdafx.h"
#include "ProtocolClient.h"
#include "Utils/ErrorsSubsystem.h"


using namespace ESS;


namespace BfBoot
{
    //-------------------------------------------------------------------------
    // ProtocolClient implementation
    //-------------------------------------------------------------------------


    ProtocolClient::ProtocolClient(IProtocolClientToUser &observer) :
        m_pTransport(0),
        m_observer(observer)
    {
    }


    //-------------------------------------------------------------------------


    ProtocolClient::~ProtocolClient()
    {
    }


    //-------------------------------------------------------------------------


    void ProtocolClient::BindTransport(ITransportClient *pTransport)
    {
        ESS_ASSERT(0 == m_pTransport);
        ESS_ASSERT(0 != pTransport);

        m_pTransport = pTransport;
    }
        
        
    //-------------------------------------------------------------------------


    void ProtocolClient::Connect()
    {
        ESS_ASSERT(0 != m_pTransport);

        m_pTransport->Connect();
    }



    //-------------------------------------------------------------------------


    void ProtocolClient::Disconnect()
    {
        // todo: 
        if (!m_pTransport) ESS_ASSERT(false);

        m_pTransport->Disconnect();
    }


    //-------------------------------------------------------------------------


    void ProtocolClient::SendRequest(/* todo: arguments */)
    {
        // todo: Формирование потока
        if (!m_pTransport) ESS_ASSERT(false);

        // m_pTransport->SendRequest(data);
    }


    //-------------------------------------------------------------------------


    void ProtocolClient::OnConnected(ITransportClient &sender)
    {
        // todo: 

        m_observer.OnConnected(*this);
    }


    //-------------------------------------------------------------------------


    void ProtocolClient::OnDisconnected(ITransportClient &sender)
    {
        // todo:

        m_observer.OnDisconnected(*this);
    }


    //-------------------------------------------------------------------------


    void ProtocolClient::OnDataReceived(ITransportClient &sender, boost::shared_ptr<Packet> data)
    {
        // todo: Парсинг, таймаут

        // m_observer.OnResponseReceived(*this, data);
    }


    //-------------------------------------------------------------------------


    void ProtocolClient::OnError(ITransportClient &sender/* todo: Параметры */)
    {
        // todo: 

        // m_observer.OnError(*this);
    }

}  // namespace BfBoot
