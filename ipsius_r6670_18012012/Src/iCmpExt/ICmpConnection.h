#ifndef _I_CMP_CONNECTION_H_
#define _I_CMP_CONNECTION_H_

#include "Utils/HostInf.h"
#include "SafeBiProtoExt/SbpMsgSender.h"

namespace iCmp	{ class IPcSideRecv; }

namespace iCmpExt
{

	class ChDataCapture;

	class ICmpConnection : public Utils::IBasicInterface
	{
	public:
		virtual bool IsCmpActive() const = 0;

		virtual void ResetConnection(const std::string &reason) = 0;

		virtual Utils::HostInf BoardHost() const = 0;

		virtual const std::string& NameBoard() const = 0;

		virtual SBProtoExt::SbpMsgSender SbpSender(iCmp::IPcSideRecv *receiver = 0) = 0;

		virtual iCmpExt::ChDataCapture* CreateChDataCapture(int channel, 
			const std::string &ownerDesc) = 0;

	};


	class ICmpConEvents : public Utils::IBasicInterface
	{
	public:
		virtual void CmpActivated(const ICmpConnection *connection) = 0;

		virtual void CmpDeactivated(const ICmpConnection *connection, const std::string &desc) = 0;

		virtual void OnBfTdmEvent(const ICmpConnection *connection,
			const std::string &src, const std::string &event, const std::string &params) = 0;

		// E1
		virtual void OnBfHdlcRecv(const ICmpConnection *connection,
			const std::vector<Platform::byte> &pack) = 0;

		// AOZ
		virtual void OnBfAbEvent(const ICmpConnection *connection,
			const std::string &deviceName, int chNumber, 
			const std::string &eventName, const std::string &eventParams) = 0;
	};

};

#endif

