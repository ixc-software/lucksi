#include "stdafx.h"
#include "TransportTCPIPClient.h"


using namespace Platform;


namespace BfBoot
{


    //-------------------------------------------------------------------------
    // TransportTCPIPClient implementation
    //-------------------------------------------------------------------------


    TransportTCPIPClient::TransportTCPIPClient(const std::string &ipAddress, word port, ITransportClientToProtocol &observer) :
        ITransportClient(),
        m_observer(observer),
        m_ipAddress(ipAddress),
        m_port(port)
    {
    }


    //-------------------------------------------------------------------------


    TransportTCPIPClient::~TransportTCPIPClient()
    {
        // todo: Disconnect without event generation (if needed)
    }


    //-------------------------------------------------------------------------


    void TransportTCPIPClient::Connect()
    {
        // todo:
    }


    //-------------------------------------------------------------------------


    void TransportTCPIPClient::Disconnect()
    {
        // todo:
    }


    //-------------------------------------------------------------------------


    void TransportTCPIPClient::SendData(boost::shared_ptr<Packet> data)
    {
        // todo:
    }


} // namespace BfBoot
