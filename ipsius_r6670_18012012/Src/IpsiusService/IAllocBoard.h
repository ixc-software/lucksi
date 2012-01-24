#ifndef IALLOCBOARD_H
#define IALLOCBOARD_H

#include "stdafx.h"
#include "Utils/IBasicInterface.h"
#include "BfBootCore/BroadcastMsg.h"

namespace IpsiusService
{
    class IAllocBoard : public Utils::IBasicInterface
    {
    public:
        virtual void BoardAllocated(QString alias, const BfBootCore::BroadcastMsg &msg) = 0;
        virtual void BoardUnallocated(QString alias) = 0;
    };
} // namespace IpsiusService

#endif
