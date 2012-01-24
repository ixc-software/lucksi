#pragma once

#include "Utils/IBasicInterface.h" 
#include "Utils/SafeRef.h" 
#include "Platform/PlatformTypes.h"

namespace Dss1ToSip
{
    class IL1ToHardware;

    class IHardwareToL1 : public Utils::IBasicInterface
    {
    public:
		virtual void LinkHardware(Utils::SafeRef<IL1ToHardware>) = 0;
		virtual void UnlinkHardware(const IL1ToHardware*) = 0;
        virtual void Activated() = 0;
        virtual void Deactivated() = 0;
        virtual void DataInd(const std::vector<Platform::byte> &pack) = 0;
    };

    class IL1ToHardware : public Utils::IBasicInterface
    {
    public:
        virtual void DataReq(const IHardwareToL1*, const QVector<Platform::byte> packet) = 0;
    };
}


