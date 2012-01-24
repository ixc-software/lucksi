#ifndef __IUDP_SOCKET_TO_USER_H__
#define __IUDP_SOCKET_TO_USER_H__

#include "stdafx.h"
#include "ISocketId.h"
#include "Utils/IBasicInterface.h"

namespace iNet
{
    class SocketData;
    class SocketError;

    // ��������� ������������ UDP ������
    class IUdpSocketToUser : public Utils::IBasicInterface
    {
    public:
        // ��������� ��������� ������ ������
        virtual void ReceiveData(boost::shared_ptr<SocketData> data) = 0;

        // ��������� ������
        virtual void SocketErrorOccur(boost::shared_ptr<SocketError> error) = 0;

    };
};
#endif

