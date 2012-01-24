#include "stdafx.h"

#include "SocketData.h"
#include "SocketError.h"
#include "UdpSocket.h"
#include "MsgUdpSocket.h"

namespace iNet 
{

    MsgUdpSocket::MsgUdpSocket(iCore::MsgThread &thread, SafeRef<IUdpSocketToUser> iUdpSocketToUser, 
        const Utils::HostInf &host) :
        iCore::MsgObject(thread),
        m_socketId(this), 
        m_user(iUdpSocketToUser),
        m_udpSocket(new UdpSocket(this, host)){}
    
    // --------------------------------------------

    MsgUdpSocket::MsgUdpSocket(iCore::MsgThread &thread, SafeRef<IUdpSocketToUser> iUdpSocketToUser) :
        iCore::MsgObject(thread),
        m_socketId(this),
        m_user(iUdpSocketToUser),
        m_udpSocket(new UdpSocket(this)){}

    // --------------------------------------------

    bool MsgUdpSocket::Bind(const Utils::HostInf &host)
    {
        ESS_ASSERT(!host.Empty());
        return m_udpSocket->Bind(host);
    }

    // --------------------------------------------

    bool MsgUdpSocket::IsEqual(SocketId id) const
    {
        return id == ID();
    }
    
    // --------------------------------------------

    SocketId MsgUdpSocket::ID() const
    {
        return m_socketId;
    }

    // --------------------------------------------
    
    void MsgUdpSocket::SendData(boost::shared_ptr<SocketData> data)
    {
        m_udpSocket->SendData(data);
    }
    
    // --------------------------------------------
    
    const Utils::HostInf &MsgUdpSocket::LocalInf() const          
    {
        return m_udpSocket->LocalInf();
    }

    // --------------------------------------------

    void MsgUdpSocket::ReceiveData(boost::shared_ptr<SocketData> data)
    {
        PutMsg(this, &T::MsgReceiveData, data);
    }

    // --------------------------------------------

    void MsgUdpSocket::SocketErrorOccur(boost::shared_ptr<SocketError> error)
    {
        PutMsg(this, &T::MsgSocketErrorOccur, error);
    }

    // --------------------------------------------

    void MsgUdpSocket::MsgReceiveData(boost::shared_ptr<SocketData> data)
    {
        data->setSocketId(ID());
        m_user->ReceiveData(data);
    }
    
    // --------------------------------------------
    
    void MsgUdpSocket::MsgSocketErrorOccur(boost::shared_ptr<SocketError> error)
    {
        error->setSocketId(ID());
        m_user->SocketErrorOccur(error);
    }
};
