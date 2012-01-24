#ifndef _BOOT_SERVER_CMD_SOURCE_H_
#define _BOOT_SERVER_CMD_SOURCE_H_

#include "Utils/IBasicInterface.h"
#include "Utils/SafeRef.h"

namespace SBProto
{
    class ISafeBiProtoForSendPack;
    class SbpRecvPack;
    class SbpError;
};

namespace BfBootSrv
{
    class IBootCmdSourceEvents;

    class IBootCmdSource : public Utils::IBasicInterface
    {
    public:
        virtual void Connected(const IBootCmdSourceEvents *) = 0;
        virtual void Disconnected(const IBootCmdSourceEvents *, const std::string &str) = 0;

        virtual SBProto::ISafeBiProtoForSendPack &Proto() = 0;
        virtual std::string ToString() const = 0;
    };

    class IBootCmdSourceEvents : public Utils::IBasicInterface
    {
    public:
        virtual bool CheckPasword(const std::string &pwd) const = 0;
        virtual void Connect(Utils::SafeRef<IBootCmdSource>) = 0;
        
        virtual void CommandReceived(const IBootCmdSource *,
            boost::shared_ptr<SBProto::SbpRecvPack> data) = 0;
        virtual void ResponceReceived(const IBootCmdSource *,
            boost::shared_ptr<SBProto::SbpRecvPack> data) = 0;
        virtual void InfoReceived(const IBootCmdSource *,
            boost::shared_ptr<SBProto::SbpRecvPack> data) = 0;
        virtual void ProtocolError(const IBootCmdSource *,
            boost::shared_ptr<SBProto::SbpError> err) = 0;
        virtual void TransportDisconnected(const IBootCmdSource *,
            const std::string &) = 0;
    };
};

#endif

