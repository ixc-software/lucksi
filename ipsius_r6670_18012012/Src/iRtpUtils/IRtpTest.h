#ifndef _I_TEST_RTP_SIP_
#define _I_TEST_RTP_SIP_

#include "stdafx.h"
#include "Utils/ErrorsSubsystem.h"
#include "Utils/IBasicInterface.h"
#include "iRtp/IRtpPCToUser.h"
#include "iRtp/RtpHeaderForUser.h"
#include "iMedia/iCodec.h"

namespace iRtpUtils
{
	class IRtpTest : public Utils::IBasicInterface
	{
	public:
		ESS_TYPEDEF(RtpTestError);
		virtual void StartSend(const iMedia::Codec &sendCodec) = 0;
		virtual void StartReceive(const iMedia::Codec &receiveCodec) = 0;
		virtual void ReceiveData(iRtp::SharedBdirBuff buff, iRtp::RtpHeaderForUser header) = 0;
	};
};

#endif
