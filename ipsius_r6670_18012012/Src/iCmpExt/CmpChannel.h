#ifndef _CMP_CHANNEL_H_
#define _CMP_CHANNEL_H_

#include "ICmpConnection.h"
#include "ICmpChannel.h"
#include "CmpPointLog.h"

namespace iCmpExt
{
	class ChDataCapture;

	class CmpChannel : boost::noncopyable,
		public ICmpChannel
	{
		typedef CmpChannel T;
	public:
		CmpChannel(CmpChannelId channelId,
			const std::string &boardName,
			int channelNumber,
			Utils::WeakRef<ICmpConnection&> cmpConnection, 
			const CmpChannelSettings &settings,
			ICmpChannelEvent &user,
			iLogW::ILogSessionCreator &logCreator);

		~CmpChannel();
		
		Utils::WeakRef<CmpChannel&> SelfRef();

		void DtmfEvent(int channelNumber, const std::string &event);

		void DeleteChannel(int channelNumber, const std::string &event);

	// ICmpChannel
	private:

		int ChannelNumber() const		{	return m_channelNumber;	}
        bool UseAlaw() const{	return m_settings.UseAlaw;	}
		const Utils::HostInf &ChannelRtpAddr() const{	return m_settings.ChannelRtpAddr;	}

		void StartRecv();
		void StopRecv();
		void StartSend(const Utils::HostInf &remoteRtp);
		void StopSend(const Utils::HostInf &remoteRtp);
		void StopSend();
		void SendTone(const iDSP::Gen2xProfile &tone, bool toLine);
		void StopTone(bool toLine);
		void StartRing(int pulseTime, int silenceTime);
		void StopRing();
		void EchoSuppress(int echoTape, bool useInternal);
		void DtmfDetect(bool useInternal, iCmp::FreqRecvMode mode);

	private:
		void ErrorOccur(const std::string &data);
		SBProtoExt::SbpMsgSender Sender();
		void StopAll();
		void EchoSuppressImpl(int echoTape, bool useInternal);

	private:
		CmpChannelId m_channelId;
		std::string m_boardName;
		int m_channelNumber;
		Utils::WeakRef<ICmpConnection&> m_cmpConnection;
		Utils::HostInf m_remoteRtp;

		CmpChannelSettings m_settings;

		ICmpChannelEvent &m_user;
		bool m_isReceiveOn;

		bool m_isToneSendToLine;
		bool m_isToneSendFromLine;

		bool m_isRingOn;
		class ResponseReceiver;
		Utils::WeakRefHost m_selfRefHost;
		CmpPointLog m_log;
		boost::scoped_ptr<ChDataCapture> m_chDataCapture;
	};
};

#endif




