#include "stdafx.h"
#include "TransportUARTServer.h"


using namespace Platform;


namespace BfServer
{

    //-------------------------------------------------------------------------
    // TransportUARTServer implementation
    //-------------------------------------------------------------------------


    TransportUARTServer::TransportUARTServer(word comPort, Executer &executer, ITransportServerEvents &observer) :
        ExecutionUnit(executer),
        m_observer(observer),
        m_comPort(comPort)
    {
    }


    //-------------------------------------------------------------------------


    TransportUARTServer::~TransportUARTServer()
    {
    }


    //-------------------------------------------------------------------------


    void TransportUARTServer::SendData(boost::shared_ptr<Packet> data)
    {
        // todo: 
    }


    //-------------------------------------------------------------------------
    // ExecutionUnit implementation

    void TransportUARTServer::OnExecute()
    {
        // todo: 
    }

} // namespace BfServer
