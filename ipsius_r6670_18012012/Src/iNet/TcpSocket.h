#ifndef __TCP_SOCKET_H__
#define __TCP_SOCKET_H__

#include "stdafx.h"

#include "Utils/HostInf.h"
#include "ISocketId.h"
#include "ITcpSocket.h"

namespace iNet
{
    class ITcpSocketToUser;

    // Базовый класс TCP сокетов
    class TcpSocket : boost::noncopyable,
        public ITcpSocket
    {
        friend class TcpServer;
        TcpSocket();
        static boost::shared_ptr<ITcpSocket> CreateTcpSocket(int socketDescriptor);

        bool InitSocket(int socketDescriptor);

        SocketId ID() const
        {
            return m_socketId;
        }
        SocketId m_socketId;

    // Implementation TcpSocket
    private:

        class Impl;
        Impl *m_impl;

    public:

        TcpSocket(Utils::SafeRef<ITcpSocketToUser> iTcpSocketToUser);
        ~TcpSocket();

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

