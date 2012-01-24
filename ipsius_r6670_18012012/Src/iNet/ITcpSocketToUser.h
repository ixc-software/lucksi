#ifndef __ITCP_SOCKET_TO_USER_H__
#define __ITCP_SOCKET_TO_USER_H__

#include "stdafx.h"
#include "Utils/IBasicInterface.h"

#include "ISocketId.h"

namespace iNet
{
    class SocketData;
    class SocketError;

    // Интерфейс пользователя TCP сокета
    class ITcpSocketToUser : public Utils::IBasicInterface
    {
    public:
        // This event is emitted after connectToHost() has been called
        // and a connection has been successfully established.
        virtual void Connected(SocketId id) = 0;

        // индикация получения пакета данных
        virtual void ReceiveData(boost::shared_ptr<SocketData> data) = 0;

        // This event is emitted when the socket has been disconnected.
        virtual void Disconnected(boost::shared_ptr<SocketError> error) = 0;

    };
};

#endif

