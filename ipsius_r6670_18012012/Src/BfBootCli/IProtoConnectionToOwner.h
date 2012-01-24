#ifndef IPROTOCONNECTIONTOOWNER_H
#define IPROTOCONNECTIONTOOWNER_H

#include "Utils/IBasicInterface.h"

namespace BfBootCli
{
    class IProtoConnectionToOwner : public Utils::IBasicInterface
    {
    public:
        virtual void DiscWithProtoError(const std::string& info) = 0;
        virtual void Connected() = 0;
    };
} // namespace BfBoolCli

#endif
