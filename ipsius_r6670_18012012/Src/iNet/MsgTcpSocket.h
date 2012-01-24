#ifndef __MSG_TCP_SOCKET_H__
#define __MSG_TCP_SOCKET_H__

#include "stdafx.h"

#include "iCore/MsgThread.h"
#include "iCore/MsgObject.h"
#include "ISocketId.h"
#include "Utils/HostInf.h"
#include "ITcpSocket.h"
#include "ITcpSocketToUser.h"

namespace iNet 
{
    class SocketData;    
    class SocketError;
    class TcpSocket;
    class MsgTcpServer;

    // Tcp сокет, работающий через очередь особщений
    class MsgTcpSocket : 
        boost::noncopyable,
        public iCore::MsgObject, 
        public ISocketId,
        public ITcpSocket,
        public ITcpSocketToUser 
    {
    // интерфейс ITcpSocketToUser 
    private:
        typedef MsgTcpSocket T;        
        
        // This event is emitted after connectToHost() has been called 
        // and a connection has been successfully established.
        void Connected(SocketId id);

        // индикация получения пакета данных
        void ReceiveData(boost::shared_ptr<SocketData> socketData);

        // This event is emitted when the socket has been disconnected.
        void Disconnected(boost::shared_ptr<SocketError> error);

    private:    
        friend class MsgTcpServer;
        MsgTcpSocket(iCore::MsgThread &thread, boost::shared_ptr<ITcpSocket> tcpSocket); 

        boost::shared_ptr<ITcpSocket> m_tcpSocket; 

        Utils::SafeRef<ITcpSocketToUser> m_user; 

        SocketId m_socketId;
        SocketId ID() const;

        Utils::HostInf m_localInf;          
        Utils::HostInf m_peerInf;  
        QString m_peerName;


    // обертка интерфейса ITcpServerToUser сообщениями 
    private:    
        void MsgConnected();
        void MsgReceiveData(boost::shared_ptr<iNet::SocketData> socketData);
        void MsgDisconnected(boost::shared_ptr<SocketError> error);
        void MsgSocketErrorOccur(boost::shared_ptr<iNet::SocketError> socketError);

    public:
        
        MsgTcpSocket(iCore::MsgThread &thread, Utils::SafeRef<ITcpSocketToUser> iTcpSocketToUser);

    // реализация ITcpSocket
    public:
        bool IsEqual(SocketId id) const;  
        void LinkUserToSocket(Utils::SafeRef<ITcpSocketToUser> iTcpSocketToUser);

        void ConnectToHost(const Utils::HostInf &host);

        void SendData(const QByteArray &data);
        
        void DisconnectFromHost();

        const Utils::HostInf &LocalHostInf() const;          
        const Utils::HostInf &PeerHostInf() const;  

        QString getPeerName () const;

        // end of реализация ITcpSocket
    };
};

#endif

