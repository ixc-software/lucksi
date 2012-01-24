#ifndef _SBP_TCP_SERVER_BASE_H_
#define _SBP_TCP_SERVER_BASE_H_

#include "stdafx.h"
#include "Utils/HostInf.h"
#include "ISbpTransportFactory.h"


namespace iCore {	class MsgThread;	};

namespace SBProto
{
	class SbpTcpServerBase;
    class ISbpTransport;

	class ISbpTcpServerBaseEvents : public Utils::IBasicInterface
	{
	public:
		virtual void NewConnection(const SbpTcpServerBase *, 
            boost::shared_ptr<ISbpTransportFactory>) = 0;
		virtual void Error(const SbpTcpServerBase *, const std::string &) = 0;
	};

    class SbpTcpServerBase : boost::noncopyable
    {
    public:
        SbpTcpServerBase(iCore::MsgThread &thread, const Utils::HostInf &host, ISbpTcpServerBaseEvents &user);
        ~SbpTcpServerBase();
        void Process();
    private:
        class Impl;
        boost::scoped_ptr<Impl> m_impl;
    };
};
#endif


