#ifndef __ITCP_SERVER_TO_USER_H__
#define __ITCP_SERVER_TO_USER_H__

#include "stdafx.h"

#include "Utils/SafeRef.h"
#include "Utils/IBasicInterface.h"
#include "ISocketId.h"

namespace iNet
{
    class ITcpSocket;
    class ITcpServerToUser : public Utils::IBasicInterface
    {
    public:
        virtual void NewConnection(SocketId id, boost::shared_ptr<ITcpSocket> socket) = 0;
    };

};

#endif

