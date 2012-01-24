#ifndef IBOOTUPDATER_H
#define IBOOTUPDATER_H

#include "Utils/IBasicInterface.h"
#include "BfBootCore/BroadcastMsg.h"

namespace IpsiusService
{
    class IBootUpdater : Utils::IBasicInterface
    {
    public:
        //virtual void Update(const Utils::HostInf& addr) = 0;
        virtual void Update(const BfBootCore::BroadcastMsg& msg) = 0;
    };
} // namespace IpsiusService

#endif
