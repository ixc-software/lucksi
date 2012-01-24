
#include "stdafx.h"

#include "TelnetSession.h"
#include "TelnetCommands.h"
#include "TelnetHelpers.h"

namespace Telnet
{
    // state handlers
    void TelnetSession::ChangeState(TelnetSession::States state)
    {
        m_state = state;
    }

    // ------------------------------------------
    // implement ITcpSocketToUser
    void TelnetSession::Connected(iNet::SocketId id)
    {
        ChangeState(ConnectedSt);
        StartSession();
    }

    // ------------------------------------------

    void TelnetSession::ReceiveData(shared_ptr<iNet::SocketData> data)
    {
        m_collector.ProcessSocketData(data->getData());
    }

    // ------------------------------------------

    void TelnetSession::Disconnected(boost::shared_ptr<iNet::SocketError> error)
    {
        ChangeState(DisconnectedSt);
        SendToOwnerDiscInd();

        if (error == 0) return;

        if (error->getErrorString().isEmpty()
            || error->getErrorIndex() == QAbstractSocket::RemoteHostClosedError)
        {
            // not an error
            return; 
        }

        std::string info = error->getErrorString().toStdString();
        ESS_THROW_MSG(SocketErrorInfo, info);
    }

    // ------------------------------------------

    void TelnetSession::SocketErrorOccur(shared_ptr<iNet::SocketError> error)
    {
        SendToOwnerSocketErrorInd(error);
    }

    // ------------------------------------------
    // implement ITelnetDataCollectorEvents
    // socket <--> telnet
    void TelnetSession::SendToSocketCmd(byte cmd, byte opt)
    {
        if (cmd == CMD_NOP) return;

        QByteArray res(TH::MakeCmd(cmd, opt));
        SendBytes(res);
        // m_logger.Add(res, false, TelnetDataLogger::CMD_3b);
        GetLogger().Add(res, false, TelnetDataLogger::CMD_3b); // output
    }
    
    // ------------------------------------------

    void TelnetSession::SendToSocketCmd(byte cmd)
    {
        if (cmd == CMD_NOP) return;

        QByteArray res(TH::MakeCmd(cmd));
        SendBytes(res);
        // m_logger.Add(res, false, TelnetDataLogger::CMD_2b);
        GetLogger().Add(res, false, TelnetDataLogger::CMD_2b); // output
    }

    // ---------------------------------------------------

    byte TelnetSession::At(const QByteArray &array, size_t index)
    {
        return TelnetHelpers::At(array, index);
    }

    // ---------------------------------------------------

    void TelnetSession::Append(QByteArray &array, byte val)
    {
        TelnetHelpers::Append(array, val);
    }

    // ------------------------------------------
    // messages
    void TelnetSession::EstablishReq(const Utils::HostInf &host)
    {
        PutMsg(this, 
               &T::OnEstablishReq, 
               boost::shared_ptr<Utils::HostInf>(new Utils::HostInf(host)));
    }

    // ------------------------------------------

    void TelnetSession::DisconnectReq()
    {
        PutMsg(this, &T::OnDisconnectReq);
    }

    // ------------------------------------------

    void TelnetSession::SendBytes(const QByteArray &data, bool logIt)
    {
        PutMsg(this, &T::OnSendBytes, 
               shared_ptr<QByteArray>(new QByteArray(data)));
    }

    // ------------------------------------------
    // msg events
    void TelnetSession::OnEstablishReq(boost::shared_ptr<Utils::HostInf> pHostInf)
    {
        // if socket already connected
        //if ((pHostInf->getAddress().isNull()) || (m_state == ConnectedSt)) return;

        ESS_ASSERT(!pHostInf->Empty());

        if (m_state == EstablishedSt) return;
        m_pSocket->ConnectToHost(*pHostInf);
    }

    // ------------------------------------------

    void TelnetSession::OnDisconnectReq()
    {
        ChangeState(DisconnectedSt);
        m_pSocket->DisconnectFromHost();
    }

    // ------------------------------------------
    // send to socket
    void TelnetSession::OnSendBytes(shared_ptr<QByteArray> pData)
    {   
        ESS_ASSERT(m_pSocket.get() != 0);

        /*
        ESS_ASSERT((m_state == ConnectedSt) 
                   || (m_state == EstablishedSt));
        */

        if (m_state == DisconnectedSt) return;
        
        m_pSocket->SendData(*pData);
    }

} // namesapce Telnet

