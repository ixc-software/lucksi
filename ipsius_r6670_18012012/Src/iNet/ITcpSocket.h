#ifndef __ITCP_SOCKET_H__
#define __ITCP_SOCKET_H__

#include "stdafx.h"

#include "Utils/IBasicInterface.h"
#include "Utils/HostInf.h"
#include "ISocketId.h"

namespace iNet
{

    class ITcpSocketToUser;

    // Интерфейс TCP сокета
    class ITcpSocket : public Utils::IBasicInterface
    {
    public:
        virtual bool IsEqual(SocketId id) const = 0;

        virtual void LinkUserToSocket(Utils::SafeRef<ITcpSocketToUser> iTcpSocketToUser) = 0;

        void ConnectToHost(const std::string &hostName, int port)
        {
            ConnectToHost(Utils::HostInf(hostName, port));
        }

        virtual void ConnectToHost(const Utils::HostInf &host) = 0;

        virtual void SendData(const QByteArray &data) = 0;

        virtual void DisconnectFromHost() = 0;

        virtual const Utils::HostInf &LocalHostInf() const = 0;
        virtual const Utils::HostInf &PeerHostInf() const = 0;

        virtual QString getPeerName () const = 0;

        // end of интерфейс TcpSocket
    };
};
#endif

