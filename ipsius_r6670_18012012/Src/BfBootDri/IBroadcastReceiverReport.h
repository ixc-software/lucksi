#ifndef IBROADCASTRECEIVERREPORT_H
#define IBROADCASTRECEIVERREPORT_H

#include "BfBootCore/BroadcastMsg.h"

namespace BfBootDRI
{
    class IBroadcastReceiverReport : Utils::IBasicInterface
    {
    public:
        virtual void ExpectedMessageReceived(const BfBootCore::BroadcastMsg& msg) = 0;
        virtual void OnTimeout() = 0;
    };
} // namespace BfBootDRI

#endif
