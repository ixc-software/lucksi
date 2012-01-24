#ifndef _I_CMP_DTMF_TEST_H_
#define _I_CMP_DTMF_TEST_H_

#include "Utils/IBasicInterface.h"
#include "Utils/HostInf.h"

namespace iCmpExt
{
	class IDtmfTestCmd;

	class IDtmfTestEvent : public Utils::IBasicInterface
	{
	public:
		virtual void DtmfEvent(const IDtmfTestCmd*, const std::string &) = 0;
		virtual void Error(const IDtmfTestCmd*, const std::string &desc) = 0;
	};

	class IDtmfTestCmd : public Utils::IBasicInterface
	{
	public:
		virtual void StartDtmfDetect(int channel) = 0;
		virtual void StartRtpReceiver(int channel) = 0;		
		Utils::HostInf ChannelRtpAddr(int channel) const = 0;
	};

	class IDtmfTestCreator : public Utils::IBasicInterface
	{
	public:
		boost::shared_ptr<IDtmfTestCmd> CreateDtmfTest(IDtmfTestEvent &user)  = 0;
	};
};

#endif




