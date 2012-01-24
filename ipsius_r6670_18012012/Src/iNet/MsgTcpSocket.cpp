#include "stdafx.h"

#include "SocketData.h"
#include "SocketError.h"
#include "TcpSocket.h"
#include "MsgTcpSocket.h"

namespace iNet
{
    MsgTcpSocket::MsgTcpSocket(iCore::MsgThread &thread, Utils::SafeRef<ITcpSocketToUser> iTcpSocketToUser) :
        iCore::MsgObject(thread),
        m_tcpSocket(new TcpSocket(this)),
        m_user(iTcpSocketToUser),
        m_socketId(this)
	{
            m_localInf = m_tcpSocket->LocalHostInf();
	}

    // ---------------------------------------------

    MsgTcpSocket::MsgTcpSocket(iCore::MsgThread &thread, boost::shared_ptr<ITcpSocket> tcpSocket):
        iCore::MsgObject(thread),
        m_tcpSocket(tcpSocket),
        m_socketId(this)
        {
            m_tcpSocket->LinkUserToSocket(this);

            m_localInf = m_tcpSocket->LocalHostInf();          
            m_peerInf = m_tcpSocket->PeerHostInf();  
            m_peerName = m_tcpSocket->getPeerName();
        }
    
    // ---------------------------------------------    

    bool MsgTcpSocket::IsEqual(SocketId id) const  
    {
        return id == ID();
    }

    // ---------------------------------------------    

    SocketId MsgTcpSocket::ID() const
    {
        return m_socketId;
    }
    
    // ---------------------------------------------    
    
    void MsgTcpSocket::Connected(SocketId id)
    {
        PutMsg(this, &T::MsgConnected);
    }

    // ---------------------------------------------

    void MsgTcpSocket::ReceiveData(boost::shared_ptr<SocketData> data)
    {
        PutMsg(this, &T::MsgReceiveData, data);
    }

    // ---------------------------------------------

    void MsgTcpSocket::Disconnected(boost::shared_ptr<SocketError> error)
    {
        PutMsg(this, &T::MsgDisconnected, error);
    }

    // ---------------------------------------------

    void MsgTcpSocket::MsgConnected()
    {
        m_peerInf = m_tcpSocket->PeerHostInf();  
        m_peerName = m_tcpSocket->getPeerName();
        m_user->Connected(ID());
    }

    // индикация получения пакета данных
    void MsgTcpSocket::MsgReceiveData(boost::shared_ptr<iNet::SocketData> data)
    {
        data->setSocketId(ID());
        m_user->ReceiveData(data);
    }

    // ---------------------------------------------

    // This event is emitted when the socket has been disconnected.
    void MsgTcpSocket::MsgDisconnected(boost::shared_ptr<SocketError> socketError)
    {
        m_peerInf.Clear();
        m_peerName.clear();
        
        socketError->setSocketId(ID());
        m_user->Disconnected(socketError);
    }

    // ---------------------------------------------
    // связывание с пользователем

    void MsgTcpSocket::LinkUserToSocket(Utils::SafeRef<ITcpSocketToUser> iTcpSocketToUser)
    {
        m_user = iTcpSocketToUser;
    }

    // ---------------------------------------------
    // интерфейс TcpSocket

    void MsgTcpSocket::ConnectToHost(const Utils::HostInf &host)
    {
        ESS_ASSERT(!m_user.IsEmpty());

        m_tcpSocket->ConnectToHost(host);
    }

    // ---------------------------------------------

    void MsgTcpSocket::SendData(const QByteArray &data)
    {
        ESS_ASSERT(!m_user.IsEmpty());

        m_tcpSocket->SendData(data);
    }

    // ---------------------------------------------

    void MsgTcpSocket::DisconnectFromHost()
    {
        ESS_ASSERT(!m_user.IsEmpty());

        m_tcpSocket->DisconnectFromHost();
    }

    // ---------------------------------------------

    const Utils::HostInf &MsgTcpSocket::LocalHostInf() const
    {
        return m_localInf;
    }

    // ---------------------------------------------

    const Utils::HostInf &MsgTcpSocket::PeerHostInf() const
    {
        return m_peerInf;
    }

    // ---------------------------------------------

    QString MsgTcpSocket::getPeerName() const
    {
        return m_peerName;
    }

    // end of интерфейс TcpSocket

};
