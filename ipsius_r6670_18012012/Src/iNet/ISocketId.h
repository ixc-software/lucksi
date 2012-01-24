#ifndef __ISOCKET_ID_H__
#define __ISOCKET_ID_H__

#include "stdafx.h"

#include "Utils/SafeRef.h"
#include "Utils/IBasicInterface.h"

namespace iNet
{

    class ISocketId : public Utils::IBasicInterface
    {
    public:
    };

    typedef Utils::SafeRef<ISocketId> SocketId;
};

#endif

