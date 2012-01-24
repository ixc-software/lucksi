#ifndef _I_RTP_SIP_TEST_FACTORY_H_
#define _I_RTP_SIP_TEST_FACTORY_H_

#include "stdafx.h"
#include "Utils/IBasicInterface.h"
#include "Utils/SafeRef.h"

namespace iCore {   class MsgThread;    };
namespace iRtp  {   class RtpPcSession; };

namespace iRtpUtils
{
    class IRtpTest;
	class IRtpTestFactory : public Utils::IBasicInterface
	{
	public:
		virtual IRtpTest *CreateTest(iCore::MsgThread& thread, 
			Utils::SafeRef<iRtp::RtpPcSession>) = 0;
	};
};

#endif
