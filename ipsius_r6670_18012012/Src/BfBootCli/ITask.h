#ifndef ITASK_H
#define ITASK_H

#include "Platform/PlatformTypes.h"
#include "BfBootCore/IClientToBooter.h"
#include "BfBootCore/IBootServerToClient.h"

#include "Utils/IBasicInterface.h"

namespace BfBootCli
{
    // интерфейс задания
    class ITask : public Utils::IBasicInterface
    {
    public:
        virtual void Run( BfBootCore::IClientToBooter& remoteServer ) = 0;        
        virtual QString Info() const = 0; // Имя + прогресс.

        virtual BfBootCore::IBootServerToClient& getClientIntf() = 0;
    };
} // namespace BfBootCli

#endif
