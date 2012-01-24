
#ifndef __ITELNETDRI__
#define __ITELNETDRI__

// ITelnetDRI.h

#include "Utils/IBasicInterface.h"
#include "Domain/IDomain.h"
#include "iLog/LogWrapper.h"

namespace iLogW
{
    class LogManager;
    
} // namespace iLogW

namespace iCore
{
    class MsgThread;
    
} // namespace iCore

namespace DRI
{
    class TelnetServerSessionDRI;

    // DRI session to NObj-wrapper callback
    class ITelnetSessionDRIToOwner : public Utils::IBasicInterface
    {
    public:
        virtual bool IsSame(const TelnetServerSessionDRI *pSession) const = 0;
        virtual QString GetInfo() const = 0;
        virtual QString GetStats() const = 0;
        virtual void Close() = 0;
        virtual QString UserName() const = 0;
    };

    // --------------------------------------------------------

    // DRI session to server callback
    class ITelnetSessionDRIToServer : public Utils::IBasicInterface
    {
    public:
        virtual void Unregister(const TelnetServerSessionDRI *pSession) = 0;
    };

} // namespace DRI

#endif 
