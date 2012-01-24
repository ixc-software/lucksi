#ifndef _I_SBP_TCP_SERVER_EVENTS_H_
#define _I_SBP_TCP_SERVER_EVENTS_H_

#include "Utils/IBasicInterface.h"
#include "Utils/SafeRef.h"
#include "ISbpConnection.h"

namespace SBProtoExt
{
    class SbpTcpServer;

    class ISbpTcpServerEvents : public Utils::IBasicInterface
    {
    public:
        virtual void NewConnection(const SbpTcpServer*,
			boost::shared_ptr<ISbpConnection>) = 0;       
        virtual void Error(const SbpTcpServer *, const std::string &) = 0;
    };
};

#endif

