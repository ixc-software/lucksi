#ifndef _I_SBP_CONNECTION_H_
#define _I_SBP_CONNECTION_H_

#include "Utils/IBasicInterface.h"
#include "Utils/SafeRef.h"

namespace SBProto
{
	class ISafeBiProtoForSendPack;
	class SbpRecvPack;
	class SbpError;
};
namespace SBProtoExt
{

    class ISbpConnectionEvents;

    class ISbpConnection : public Utils::IBasicInterface
    {
    public:
        virtual void Process() = 0;
        virtual void BindUser(Utils::SafeRef<ISbpConnectionEvents>) = 0;
		virtual void UnbindUser() = 0;
        virtual void ActivateConnection() = 0;
        virtual void DeactivateConnection() = 0;

		virtual SBProto::ISafeBiProtoForSendPack &Proto() = 0;
        virtual std::string TransportInfo() const = 0;
        virtual std::string Name() const = 0;
    };

    class ISbpConnectionEvents : public Utils::IBasicInterface
    {
    public:
        virtual void CommandReceived(Utils::SafeRef<ISbpConnection> src,
            boost::shared_ptr<SBProto::SbpRecvPack> data) = 0;
        virtual void ResponseReceived(Utils::SafeRef<ISbpConnection> src,
            boost::shared_ptr<SBProto::SbpRecvPack> data) = 0;
        virtual void InfoReceived(Utils::SafeRef<ISbpConnection> src,
            boost::shared_ptr<SBProto::SbpRecvPack> data) = 0;
        virtual void ProtocolError(Utils::SafeRef<ISbpConnection> src,
            boost::shared_ptr<SBProto::SbpError> err) = 0;

        virtual void ConnectionActivated(Utils::SafeRef<ISbpConnection> src) = 0;
        virtual void ConnectionDeactivated(Utils::SafeRef<ISbpConnection> src,
            const std::string &) = 0;
    };
};

#endif

