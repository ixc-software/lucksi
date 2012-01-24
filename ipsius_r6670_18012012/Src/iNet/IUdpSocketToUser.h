#ifndef __IUDP_SOCKET_TO_USER_H__
#define __IUDP_SOCKET_TO_USER_H__

#include "stdafx.h"
#include "ISocketId.h"
#include "Utils/IBasicInterface.h"

namespace iNet
{
    class SocketData;
    class SocketError;

    // Интерфейс пользователя UDP сокета
    class IUdpSocketToUser : public Utils::IBasicInterface
    {
    public:
        // индикация получения пакета данных
        virtual void ReceiveData(boost::shared_ptr<SocketData> data) = 0;

        // индикация ошибки
        virtual void SocketErrorOccur(boost::shared_ptr<SocketError> error) = 0;

    };
};
#endif

