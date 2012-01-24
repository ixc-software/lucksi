#include "stdafx.h"
#include "undeferrno.h"

#include "AutoTrainerSipCall.h"
#include "MediaLineList.h"
#include "MediaLine.h"
#include "SdpUtils.h"

#include "Sdp/SdpHelperResip.h"
#include "Sdp/Sdp.h"

#include "Utils/GetDefLocalHost.h"
#include "Utils/AtomicTypes.h"
#include "Utils/IntToString.h"

namespace
{

	std::string ToString(const sdpcontainer::SdpMediaLine::CodecList &codecs)
	{
		std::string out;
		
		for (sdpcontainer::SdpMediaLine::CodecList::const_iterator i = codecs.begin();
			i != codecs.end(); ++i)
		{
			out += iSip::SipUtils::ToString(*i) + "; ";
		}

		return out;
	}

	std::string CallName()
	{
		static Utils::AtomicInt GCallCounter;

		return "Call_" + Utils::IntToString(GCallCounter.FetchAndAdd(1));
	}

	iSip::MediaLineProfile CreateMediaLineProfile()
	{
		iSip::MediaLineProfile profile;

		profile.MediaType = iSip::MediaLineProfile::ML::MEDIA_TYPE_AUDIO;

		profile.LocalDirection = iSip::MediaLineProfile::ML::DIRECTION_TYPE_SENDRECV;
//			iSip::MediaLineProfile::ML::DIRECTION_TYPE_INACTIVE;

		profile.TransportProtocolType = iSip::MediaLineProfile::ML::PROTOCOL_TYPE_RTP_AVP;

		profile.PacketTime = 20;

		profile.CodecList.push_back(iSip::SdpUtils::BuildG711Codec(true));
		profile.CodecList.push_back(iSip::SdpUtils::BuildG711Codec(false));
		
		profile.LocalRtpTuple =
			iSip::MediaIpAddress(iSip::SipTransportInfo::Udp,
				boost::asio::ip::address::from_string(Utils::GetDefLocalIp().toStdString()),
				10245);
		
		return profile;
	}

}

namespace
{

	class MediaLineImpl : boost::noncopyable,
		public iSip::ISipMediaLineEvents
	{
		
		typedef MediaLineImpl T;

	public:

		MediaLineImpl(iLogW::ILogSessionCreator &logCreator,
			iSip::MediaLineList &mediaList) :
			m_log(logCreator.CreateSession("MediaLine", true)),
			m_infoTag(m_log->RegisterRecordKind(L"Info", true)),
			m_mediaList(mediaList)
		{
			if(m_log->LogActive(m_infoTag))
			{
				*m_log  << m_infoTag << "Created. " << iLogW::EndRecord;
			}

			iSip::MediaLineProfile profile = CreateMediaLineProfile();

			iSip::MediaLineList::Manager(m_mediaList).AddMediaLine(*this, profile);
		}

		~MediaLineImpl()
		{
			iSip::MediaLineList::Manager(m_mediaList).DeleteMediaLine(*this);
		}

	// iSip::ISipMediaLineEvents
	private:

		void StartRtpSend(const char *remoteIpAddr,
			unsigned int remoteRtpPort,
			unsigned int remoteRtcpPort,
			const ML::CodecList &codecs)
		{
			if(m_log->LogActive(m_infoTag))
			{
				*m_log  << m_infoTag << "StartRtpSend: "	
					<< remoteIpAddr   << " : "
					<< remoteRtpPort  << "(rtcp "
					<< remoteRtcpPort << ")"
					<< iSip::SdpUtils::ToString(codecs)
					<< iLogW::EndRecord;
			}
		}

		void StopRtpSend()
		{
			if(m_log->LogActive(m_infoTag))
			{
				*m_log  << m_infoTag << "StopRtpSend" << iLogW::EndRecord;
			}
		}

		void StartRtpReceive(const ML::CodecList &codecs)
		{
			if(m_log->LogActive(m_infoTag))
			{
				*m_log  << m_infoTag << "StartRtpReceive. "
					<< iSip::SdpUtils::ToString(codecs)
					<< iLogW::EndRecord;
			}
		}

		void StopRtpReceive()
		{
			if(m_log->LogActive(m_infoTag))
			{
				*m_log  << m_infoTag << "StopRtpReceive." << iLogW::EndRecord;
			}
		}

	private:

		boost::scoped_ptr<iLogW::LogSession>  m_log;
		iLogW::LogRecordTag m_infoTag;
		iSip::MediaLineList &m_mediaList;
	};
}

namespace iSip
{

	AutotrainerSipCall::AutotrainerSipCall(iCore::MsgThread &thread,
		iLogW::ILogSessionCreator &logCreator,
		boost::function<void (T *)> deleter,
		const Commands &commands) :
		iCore::MsgObject(thread),
		m_timer(this, &T::OnTimeout),
		m_log(logCreator.CreateSession(CallName(), true)),
		m_infoTag(m_log->RegisterRecordKind(L"Info", true)),
		m_deleter(deleter),
		m_callDeleted(false),
		m_commands(commands)
	{
		// outgoing call

		ESS_ASSERT(!m_commands.empty());

		if (m_log->LogActive(m_infoTag))
		{
			*m_log << m_infoTag << "Created" << iLogW::EndRecord;
		}

		CreateMedia();

		PutMsg(this, &T::OnCommand);
	}

	// -------------------------------------------------------------------------------------

	AutotrainerSipCall::AutotrainerSipCall(iCore::MsgThread &thread,
		iLogW::ILogSessionCreator &logCreator,
		boost::function<void (T *)> deleter,
		Utils::SafeRef<IRemoteParticipant> remoteParticipant,
		const Commands &commands) :
		iCore::MsgObject(thread),
		m_timer(this, &T::OnTimeout),
		m_log(logCreator.CreateSession(CallName(), true)),
		m_infoTag(m_log->RegisterRecordKind(L"Info", true)),
		m_deleter(deleter),
		m_callDeleted(false),
		m_commands(commands),
		m_remoteParticipant(remoteParticipant)
	{
		// Incoming call

		ESS_ASSERT(!m_commands.empty());

		if (m_log->LogActive(m_infoTag))
		{
			*m_log << m_infoTag << "Created." << iLogW::EndRecord;
		}

		m_remoteParticipant->LinkUser(this);
		
		CreateMedia();

		PutMsg(this, &T::OnCommand);
	}

	// -------------------------------------------------------------------------------------

	AutotrainerSipCall::~AutotrainerSipCall()
	{
		if (m_log->LogActive(m_infoTag))
		{
			*m_log << m_infoTag << "Deleted" << iLogW::EndRecord;
		}
	}

	// -------------------------------------------------------------------------------------

	void AutotrainerSipCall::StartOutgoingCall(Utils::SafeRef<IRemoteParticipantCreator> callCreator,
		const resip::Uri &destinationUri)
	{
		ESS_ASSERT(!callCreator.IsEmpty());

		boost::shared_ptr<IRemoteParticipantCreator::Parameters>
			parameters(new IRemoteParticipantCreator::Parameters());

		parameters->SetDestination(resip::NameAddr(destinationUri));

		parameters->SourceUserName = "3452";

		parameters->UseSourceNameInContact = true;

		parameters->UseSourceUserNameAsDisplayName = true;

		m_remoteParticipant = callCreator->CreateOutgoingParticipant(
			this,
			parameters);

		if (m_remoteParticipant.IsEmpty())
		{
			DeleteCall();
			return;
		}

		if (m_log->LogActive(m_infoTag))
		{
			*m_log << m_infoTag << "Start outgoing call. Sip call: "
				<< ". Dst: " << SipUtils::ToString(destinationUri)
				<< iLogW::EndRecord;
		}
	}

	// -------------------------------------------------------------------------------------

	void AutotrainerSipCall::RedirectTo(const resip::Uri &destinationUri)
	{
		if (m_remoteParticipant.IsEmpty()) return;

		m_remoteParticipant->RedirectParticipant(resip::NameAddr(destinationUri));

		PutMsg(this, &T::OnCommand);
	}

	// -------------------------------------------------------------------------------------

	void AutotrainerSipCall::Wait(int timeout)
	{
		m_timer.Start(timeout);
	}

	// -------------------------------------------------------------------------------------

	void AutotrainerSipCall::Alerting()
	{
		if (m_remoteParticipant.IsEmpty()) return;

		if (m_log->LogActive(m_infoTag))
		{
			*m_log << m_infoTag << "Alerting" << iLogW::EndRecord;
		}
		
		m_remoteParticipant->MediaObjectIsReady();			
		m_remoteParticipant->AlertParticipant();

		PutMsg(this, &T::OnCommand);
	}

	// -------------------------------------------------------------------------------------

	void AutotrainerSipCall::Answer()
	{
		if (m_remoteParticipant.IsEmpty()) return;

		if (m_log->LogActive(m_infoTag))
		{
			*m_log << m_infoTag << "Answer" << iLogW::EndRecord;
		}

		m_remoteParticipant->AnswerParticipant();

		PutMsg(this, &T::OnCommand);
	}

	// -------------------------------------------------------------------------------------

	void AutotrainerSipCall::Hold()
	{
		if (m_remoteParticipant.IsEmpty()) return;

		if (m_log->LogActive(m_infoTag))
		{
			*m_log << m_infoTag << "Hold" << iLogW::EndRecord;
		}

		iSip::MediaLineProfile profile = CreateMediaLineProfile();

		MediaLineList::Manager mediaManager(*m_media, m_remoteParticipant);

		mediaManager.Profile(*m_mediaLine).LocalDirection = MediaLineProfile::ML::DIRECTION_TYPE_INACTIVE;

		PutMsg(this, &T::OnCommand);
	}

	// -------------------------------------------------------------------------------------

	void AutotrainerSipCall::Unhold()
	{
		if (m_remoteParticipant.IsEmpty()) return;

		if (m_log->LogActive(m_infoTag))
		{
			*m_log << m_infoTag << "Unhold" << iLogW::EndRecord;
		}
		
		MediaLineList::Manager mediaManager(*m_media, m_remoteParticipant);
		mediaManager.Profile(*m_mediaLine).LocalDirection = MediaLineProfile::ML::DIRECTION_TYPE_SENDRECV;
		
		PutMsg(this, &T::OnCommand);
	}
	
	// -------------------------------------------------------------------------------------

	void AutotrainerSipCall::Close(int statusCode)
	{
		if (m_remoteParticipant.IsEmpty()) return;

		if (m_log->LogActive(m_infoTag))
		{
			*m_log << m_infoTag << "Close. Status code: " << statusCode << iLogW::EndRecord;
		}

		m_remoteParticipant.Clear()->DestroyParticipant(statusCode);

		PutMsg(this, &T::OnCommand);
	}

	// -------------------------------------------------------------------------------------
	// IRemoteParticipantUser

	std::string AutotrainerSipCall::CallUserName() const
	{
		return m_log->NameStr();
	}

	// -------------------------------------------------------------------------------------

	ISipMedia* AutotrainerSipCall::Media()
	{
		return m_media.get();
	}

	// -------------------------------------------------------------------------------------------------------

	boost::shared_ptr<iReg::CallRecordWrapper> AutotrainerSipCall::CallRecordByUser()
	{
		ESS_UNIMPLEMENTED;
		return boost::shared_ptr<iReg::CallRecordWrapper>();
	}

	// -------------------------------------------------------------------------------------

	const ISipMedia* AutotrainerSipCall::Media() const
	{
		return m_media.get();
	}

	// -------------------------------------------------------------------------------------

	void AutotrainerSipCall::OnParticipantTerminated(const IRemoteParticipant *src,
		unsigned int statusCode)
	{
		if (m_log->LogActive(m_infoTag))
		{
			*m_log << m_infoTag << "Call terminated. Status code: " << statusCode << iLogW::EndRecord;
		}

		DeleteCall();
	}

	// -------------------------------------------------------------------------------------

	void AutotrainerSipCall::OnRefer(const IRemoteParticipant *src, const resip::SipMessage &msg)
	{
		ESS_ASSERT (m_remoteParticipant.IsEqualIntf(src));

		if (m_log->LogActive(m_infoTag))
		{
			*m_log << m_infoTag << "Call redirected. "
				<< SipUtils::ToString(SipUtils::Short, msg)
				<< iLogW::EndRecord;
		}
	}

	// -------------------------------------------------------------------------------------

	void AutotrainerSipCall::OnParticipantAlerting(const IRemoteParticipant *src,
		const resip::SipMessage& msg)
	{

	}

	// -------------------------------------------------------------------------------------

	void AutotrainerSipCall::OnParticipantConnected(const IRemoteParticipant *src,
		const resip::SipMessage &msg)
	{

	}

	// -------------------------------------------------------------------------------------

	void AutotrainerSipCall::OnParticipantRedirectSuccess(const IRemoteParticipant *src)
	{

	}

	// -------------------------------------------------------------------------------------

	void AutotrainerSipCall::OnParticipantRedirectFailure(const IRemoteParticipant *src,
		unsigned int statusCode)
	{

	}

	// -------------------------------------------------------------------------------------

	void AutotrainerSipCall::OnForkingOccured(const IRemoteParticipant *origCall,
		Utils::SafeRef<IRemoteParticipant> newCall,
		const resip::SipMessage &msg)
	{
		newCall->DestroyParticipant();
	}

	// -------------------------------------------------------------------------------------

	void AutotrainerSipCall::InitRemoteSdp(boost::shared_ptr<resip::SdpContents> offer)
	{
		if (m_log->LogActive(m_infoTag))
		{
			*m_log << m_infoTag << "Init remote sdp: "
				<< SipUtils::ToString(*offer)
				<< iLogW::EndRecord;
		}
	}

	// -------------------------------------------------------------------------------------

	void AutotrainerSipCall::OnCommand()
	{
		if(m_commands.empty()) return;

		m_commands.front()(this);

		m_commands.pop_front();
	}

	// -------------------------------------------------------------------------------------

	void AutotrainerSipCall::OnTimeout(iCore::MsgTimer *pT)
	{
		PutMsg(this, &T::OnCommand);
	}

	// -------------------------------------------------------------------------------------

	void AutotrainerSipCall::CreateMedia()
	{
		ESS_ASSERT (m_mediaLine == 0 && m_media == 0);

		m_media.reset(new iSip::MediaLineList(
			*m_log,
			boost::bind(&T::InitRemoteSdp, this, _1)));

		m_mediaLine.reset(new MediaLineImpl(*m_log, *m_media));
	}

	// -------------------------------------------------------------------------------------

	void AutotrainerSipCall::DeleteCall()
	{
		if (m_callDeleted) return;

		m_callDeleted = true;

		Close();
		m_deleter(this);
	}

}

