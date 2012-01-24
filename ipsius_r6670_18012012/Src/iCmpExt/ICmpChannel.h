#ifndef _I_CHANNEL_TO_CMP_H_
#define _I_CHANNEL_TO_CMP_H_

#include "Utils/SafeRef.h"
#include "Utils/WeakRef.h"
#include "Utils/HostInf.h"
#include "iDSP/GenUtils.h"
#include "iCmp/ChMngProtoShared.h"

namespace iLogW {	class ILogSessionCreator; }

namespace iCmpExt
{

	typedef const void* CmpChannelId;

	// -------------------------------------------------------


	class ICmpChannel : public Utils::IBasicInterface
	{
	public:
		virtual int ChannelNumber() const = 0;
		virtual const Utils::HostInf& ChannelRtpAddr() const = 0;
		virtual bool UseAlaw() const = 0;
		virtual void StartRecv() = 0;
		virtual void StopRecv() = 0;
		virtual void StartSend(const Utils::HostInf &remoteRtp) = 0;
		virtual void StopSend(const Utils::HostInf &remoteRtp) = 0;
		virtual void StopSend() = 0;
		virtual void SendTone(const iDSP::Gen2xProfile &tone, bool toLine) = 0;
		virtual void StopTone(bool toLine) = 0;
		virtual void StartRing(int pulseTime, int silenceTime) = 0;
		virtual void StopRing() = 0;
		virtual void EchoSuppress(int echoTape, bool useInternal) = 0;
		virtual void DtmfDetect(bool useInternal, iCmp::FreqRecvMode mode) = 0;
	};

	// -------------------------------------------------------

    class ICmpChannelEvent : public Utils::IBasicInterface
	{
	public:
		virtual void ChannelCreated(ICmpChannel *cmpChannel, CmpChannelId id) = 0;
		// ICmpChannel *channel can be null
		virtual void ChannelDeleted(CmpChannelId id, const std::string &desc) = 0;
		virtual void DtmfEvent(const ICmpChannel *cmpChannel, const std::string &event) = 0;
	};

	// -------------------------------------------------------

	struct CmpChannelSettings
	{
		Utils::HostInf ChannelRtpAddr;
		int IndexNumber; // порядковый номер канала ( исп. для определения локального Rtp адреса)
		bool UseAlaw;
		int  EchoTape;
		bool UseInternalEcho; 
	};

	// -------------------------------------------------------
	
	class ICmpChannelCreator : public Utils::IBasicInterface
	{
	public:
		virtual void CreateCmpChannel(CmpChannelId id,
			int channelNumber, 
			Utils::WeakRef<ICmpChannelEvent&> channelOwner,
			Utils::WeakRef<iLogW::ILogSessionCreator&> logCreator) = 0;
	};

}

#endif
