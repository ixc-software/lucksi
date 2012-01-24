#include "stdafx.h"
#include "Sip/undeferrno.h"
#include "QueuedMeetingCall.h"
#include "CallMeetingStat.h"

#include "iSip/SipUtils.h"
#include "iSip/Sdp/SdpHelperResip.h"
#include "iSip/MediaLineList.h"
#include "iSip/MediaLine.h"

#include "Utils/GetDefLocalHost.h"
#include "Utils/AtomicTypes.h"
#include "Utils/IntToString.h"

namespace {

	const int CTryingDef	= 10;
	const int CAlertingDef	= 10;

	std::string GenerateId()
	{
		static Utils::AtomicInt GCallCounter; 

		return Utils::IntToString(GCallCounter.FetchAndAdd(1));
	}

	Platform::dword GetRandomValue(const Utils::ValueRange &range, Utils::Random &random)
	{
		return random.NextInRange(range.From(), range.To());
	}
}

// -------------------------------------------------------------------------------

namespace CallMeeting
{

    class IMediaItem : public Utils::IBasicInterface
	{
	public:

		typedef boost::function<void (boost::shared_ptr<resip::SdpContents>)>
			OfferProcessor;

		virtual bool IsRedirectedMode() const = 0;

		virtual iSip::ISipMedia& Media() = 0;

		virtual const iSip::ISipMedia& Media() const = 0;

		virtual void SetupPartnerSdp(boost::shared_ptr<resip::SdpContents> partnerSdp) = 0;

	};
}

namespace 
{

	class MediaForSimpleMode : boost::noncopyable,
		public iSip::ISipMedia,
		public CallMeeting::IMediaItem
	{
	public:

		MediaForSimpleMode(OfferProcessor offerProcessor) : 
			m_offerProcessor(offerProcessor)
		{
		}

	// IMediaItem
	public:

		bool IsRedirectedMode() const		{	return false;	}

		iSip::ISipMedia& Media()			{	return *this;	}

		const iSip::ISipMedia& Media() const{	return *this;	}

		void SetupPartnerSdp(boost::shared_ptr<resip::SdpContents> partnerSdp)
		{	
			m_partnerSdp = partnerSdp; 
		}

	// iSip::ISipMedia
	public:

		void SetupNatIpConverter(NatIpConverter converter)
		{

		}

		void InitRemoteSdp(boost::shared_ptr<resip::SdpContents> offer)
		{
			m_offerProcessor(offer);
		}

		void AdjustRTPStreams(bool isOffer)
		{

		}

		bool GenerateAnswer(resip::SdpContents &answer) const
		{
			answer = *m_partnerSdp;

			return true;
		}

		void GenerateOffer(resip::SdpContents &offer) const
		{
			offer = *m_partnerSdp;
		}

	private:

		OfferProcessor m_offerProcessor;

		boost::shared_ptr<resip::SdpContents>  m_partnerSdp;
	};

	// -------------------------------------------------------------------------------

	class MediaForRedirectMode : boost::noncopyable,
		iSip::ISipMediaLineEvents,
		public CallMeeting::IMediaItem
	{
	public:

		MediaForRedirectMode(iLogW::ILogSessionCreator &logCreator, 
			OfferProcessor offerProcessor) :
			m_mediaList(logCreator, offerProcessor)
		{
			iSip::MediaLineProfile profile;
			
			BuildInactiveProfile(profile);
			iSip::MediaLineList::Manager(m_mediaList).AddMediaLine(*this, profile);
		}


		~MediaForRedirectMode()
		{
			iSip::MediaLineList::Manager(m_mediaList).DeleteMediaLine(*this);
		}

	// IMediaItem
	public:

		bool IsRedirectedMode() const		{	return true;	}

		iSip::ISipMedia& Media()			{	return m_mediaList;	}

		const iSip::ISipMedia& Media() const{	return m_mediaList;	}

		void SetupPartnerSdp(boost::shared_ptr<resip::SdpContents> partnerSdp)
		{
			ESS_HALT ("Unexpected");
		}

	// iSip::ISipMediaLineEvents
	private:

		void StartRtpSend(const char *remoteIpAddr, 
			unsigned int remoteRtpPort, 
			unsigned int remoteRtcpPort,
			const ML::CodecList &codecs) 	{}

		void StopRtpSend()	{}

		void StartRtpReceive(const ML::CodecList &codecs)	{}

		void StopRtpReceive()	{}


	private:

		static void BuildInactiveProfile(iSip::MediaLineProfile &profile)
		{
			profile.MediaType = iSip::MediaLineProfile::ML::MEDIA_TYPE_AUDIO;

			profile.LocalDirection = iSip::MediaLineProfile::ML::DIRECTION_TYPE_INACTIVE;

			profile.TransportProtocolType = iSip::MediaLineProfile::ML::PROTOCOL_TYPE_RTP_AVP;

			profile.PacketTime = 20;

			profile.CodecList.push_back(iSip::SdpUtils::BuildG711Codec(true));
			profile.CodecList.push_back(iSip::SdpUtils::BuildG711Codec(false));

			profile.LocalRtpTuple = 
				iSip::MediaIpAddress(iSip::SipTransportInfo::Udp, 
				boost::asio::ip::address::from_string(Utils::GetDefLocalIp().toStdString()), 
				0); 
		}

	private:

		iSip::MediaLineList m_mediaList;

	};

}

namespace CallMeeting
{

	CountryFilter::QueuedCall::QueuedCall(CountryFilter &owner, 
		iLogW::ILogSessionCreator &logCreator,
		boost::shared_ptr<CallMeetingStatList> stat,
		Utils::SafeRef<iSip::IRemoteParticipant> callSip,
		const std::string &calledAddr) : 
		m_owner(owner),
		m_random(Platform::GetSystemTickCount()),
		m_calledAddr(calledAddr),
		m_log(logCreator.CreateSession(std::string("Call_") + GenerateId(), true)),
		m_infoTag(m_log->RegisterRecordKind(L"Info", true)),
		m_state(ST_TRYING),
        m_stat(stat),
		m_callSip(callSip),
		m_media(CreateMediaItem(m_owner.Profile().UseRedirect()))
	{
		ESS_ASSERT(m_owner.Profile().WaitConnectionTimeout() != 0);
		m_rejectTimer.Set(m_owner.Profile().WaitConnectionTimeout());

		m_beginCall = Utils::TimerTicks::Ticks();
		m_beginTalk = 0;

		if(m_log->LogActive(m_infoTag))
		{
			*m_log << m_infoTag << "Created. " << iLogW::EndRecord;
		}

		m_callSip->LinkUser(this);

		m_callSip->MediaObjectIsReady();

		if(m_owner.Profile().ConnectImmediately() &&
			m_owner.IsReadyCallExist())
		{
			Alerting();
		}
		else	
		{
			Trying();
		}
	}

	// -------------------------------------------------------------------------------

	CountryFilter::QueuedCall::~QueuedCall()
	{
		if(m_log->LogActive(m_infoTag))
		{
			*m_log << m_infoTag << "Deleted." << iLogW::EndRecord;
		}

		if(!m_callSip.IsEmpty()) m_callSip.Clear()->DestroyParticipant();

		if(!m_pairCall.IsEmpty()) m_pairCall.Clear()->Release(this);

		FixEndTalk();
	}

	// -------------------------------------------------------------------------------

	const std::string &CountryFilter::QueuedCall::CalledAddr() const
	{
		return m_calledAddr;
	}

	// -------------------------------------------------------------------------------

	void CountryFilter::QueuedCall::Process()
	{
		if(m_state == ST_DELETE_PROGRESS) return;

		if(m_rejectTimer.TimeOut()) 
		{
			Stat().IncRelByTimeout();
			Reject("Reject by timeout.");
			return;
		}

		if(m_owner.Profile().ConnectImmediately() &&
			m_owner.IsReadyCallExist())
		{
			if(m_state == ST_TRYING) 
			{
				Alerting();
				return;
			}
			if(m_state == ST_ALERTING) 
			{
				Ready();
				return;	
			}
		}

		if(!m_stateTimer.TimeOut()) return;

		if(m_state == ST_TRYING)
		{
			Alerting();
		}
		else if(m_state == ST_ALERTING)
		{
			Ready();
		}
		else if(m_state == ST_READY)
		{
			ReadyForOtherFilter();
		}
		else if(m_state == ST_REDIRECTED)
		{
			Stat().IncMaxTalkDuration();
			Reject("Max talk duration. Call released.");
		}
	}

	// -------------------------------------------------------------------------------

	bool CountryFilter::QueuedCall::IsActive() const
	{
		return !m_callSip.IsEmpty();
	}

	// -------------------------------------------------------------------------------

	bool CountryFilter::QueuedCall::IsReadyForOtherFilter() const
	{
		return m_state == ST_READY_OTHER_FILTER;		
	}

	// -------------------------------------------------------------------------------

	void CountryFilter::QueuedCall::redirectToParticipant(
		QueuedCall &call1, QueuedCall &call2, bool useSipRedirect)
	{
		ESS_ASSERT (&call1 != &call2);

		ESS_ASSERT (call1.IsActive() && call2.IsActive());

		ESS_ASSERT (call1.IsRedirectMode() == call2.IsRedirectMode());

		call1.SetPair(&call2, useSipRedirect);
		call2.SetPair(&call1, useSipRedirect);

		if (useSipRedirect)
		{
			Utils::SafeRef<iSip::IRemoteParticipant> dstSipCall = call2.m_callSip;

			call1.m_callSip->RedirectToParticipant(*dstSipCall.Clear()); 
		}
	}

	// -------------------------------------------------------------------------------------
	// IRemoteParticipantUser

	std::string CountryFilter::QueuedCall::CallUserName() const
	{
		return m_log->NameStr();
	}

	// -------------------------------------------------------------------------------------

	iSip::ISipMedia* CountryFilter::QueuedCall::Media()
	{
		return &m_media->Media();
	}

	// -------------------------------------------------------------------------------------

	const iSip::ISipMedia* CountryFilter::QueuedCall::Media() const
	{
		return &m_media->Media();
	}

	// -------------------------------------------------------------------------------------------------------

	boost::shared_ptr<iReg::CallRecordWrapper> CountryFilter::QueuedCall::CallRecordByUser()
	{
		ESS_UNIMPLEMENTED;
		return boost::shared_ptr<iReg::CallRecordWrapper>();
	}

	// -------------------------------------------------------------------------------------

	void CountryFilter::QueuedCall::OnParticipantTerminated(const iSip::IRemoteParticipant *src, 
		unsigned int statusCode)
	{
		if (m_log->LogActive(m_infoTag))
		{
			*m_log << m_infoTag << "Call terminated. Status code: " 
				<< statusCode 
				<< iLogW::EndRecord;
		}

		if(!m_pairCall.IsEmpty()) m_pairCall.Clear()->Release(this, statusCode);

		if(m_state != ST_REDIRECTED)
		{
			Stat().AddCancelDuration((Utils::TimerTicks::Ticks() - m_beginCall)/ 1000);
			Stat().IncCanceledCalls();
		}

		Delete();
	}

	// -------------------------------------------------------------------------------

	void CountryFilter::QueuedCall::OnRefer(const iSip::IRemoteParticipant *src, const resip::SipMessage &msg)
	{
		ESS_ASSERT (m_callSip.IsEqualIntf(src));

		if (m_log->LogActive(m_infoTag))
		{
			*m_log << m_infoTag << "Call redirected. " 
				<< iSip::SipUtils::ToString(iSip::SipUtils::Short, msg) 
				<< iLogW::EndRecord;
		}
	}


	// -------------------------------------------------------------------------------------

	void CountryFilter::QueuedCall::OnParticipantAlerting(const iSip::IRemoteParticipant *src, 
		const resip::SipMessage& msg)
	{
		ESS_HALT("not expected");
	}

	// -------------------------------------------------------------------------------------

	void CountryFilter::QueuedCall::OnParticipantConnected(const iSip::IRemoteParticipant *src,
		const resip::SipMessage &msg)
	{
		if(m_log->LogActive(m_infoTag))
		{
			*m_log << m_infoTag << "Sip call connected." << iLogW::EndRecord;
		}
	}

	// -------------------------------------------------------------------------------------

	void CountryFilter::QueuedCall::OnParticipantRedirectSuccess(const iSip::IRemoteParticipant *src)
	{
		if(m_log->LogActive(m_infoTag))
		{
			*m_log << m_infoTag << "Redirect success." << iLogW::EndRecord;
		}

	}

	// -------------------------------------------------------------------------------------

	void CountryFilter::QueuedCall::OnParticipantRedirectFailure(const iSip::IRemoteParticipant *src, 
		unsigned int statusCode)
	{
		if(m_log->LogActive(m_infoTag))
		{
			*m_log << m_infoTag << "Redirect failure. Code: " 
				<< statusCode
				<< iLogW::EndRecord;
		}


		// connect without redirecting
		redirectToParticipant(*this, *m_pairCall.Clear(), false);
	}

	// -------------------------------------------------------------------------------------

	void CountryFilter::QueuedCall::OnForkingOccured(const iSip::IRemoteParticipant *origCall, 
		Utils::SafeRef<iSip::IRemoteParticipant> newCall,
		const resip::SipMessage &msg)
	{
		newCall->DestroyParticipant();
	}

	// -------------------------------------------------------------------------------

	void CountryFilter::QueuedCall::Trying()
	{
		int timeout = GetRandomValue(m_owner.Profile().m_tryingTimeout, m_random);
		timeout = (timeout == 0) ? CTryingDef : timeout;
		
		m_stateTimer.Set(timeout);
		m_state = ST_TRYING;	
		LogState("Trying");
	}

	// -------------------------------------------------------------------------------

	void CountryFilter::QueuedCall::Alerting()
	{
		int timeout = GetRandomValue(m_owner.Profile().m_alertingTimeout, m_random);
		timeout = (timeout == 0) ? CTryingDef : timeout;

		m_stateTimer.Set(timeout);
		m_callSip->AlertParticipant(false);
			
		m_state = ST_ALERTING;	
		LogState("Alerting");
	}

	// -------------------------------------------------------------------------------

	void CountryFilter::QueuedCall::Ready()
	{
		int timeout = GetRandomValue(m_owner.Profile().m_crossFilterConnectTimeout, m_random);		
		if(timeout != 0) m_stateTimer.Set(timeout);
		
		m_state = ST_READY;
		LogState("Ready");
		m_owner.Ready(*this);		
	}


	// -------------------------------------------------------------------------------

	void CountryFilter::QueuedCall::ReadyForOtherFilter()
	{
		LogState("ReadyForAllFilter");
		
		m_state = ST_READY_OTHER_FILTER;
		m_owner.ReadyForOtherFilter(*this);
	}

	// -------------------------------------------------------------------------------

	void CountryFilter::QueuedCall::Reject(const std::string &cause)
	{
		if(m_log->LogActive(m_infoTag)) 
		{
			*m_log << m_infoTag << cause << iLogW::EndRecord;
		}
		
		if(!m_callSip.IsEmpty()) m_callSip.Clear()->DestroyParticipant();

		if(!m_pairCall.IsEmpty()) m_pairCall.Clear()->Release(this);

        Delete();
	}

	// -------------------------------------------------------------------------------

	void CountryFilter::QueuedCall::LogState(const std::string &state)
	{
		if(m_log->LogActive(m_infoTag)) 
		{
			*m_log << m_infoTag << "State: " << state << iLogW::EndRecord;
		}
	}

	// -------------------------------------------------------------------------------

	void CountryFilter::QueuedCall::SetPair(Utils::SafeRef<QueuedCall> pairCall, bool useRedirectMode)
	{
		ESS_ASSERT(m_pairCall.IsEmpty() || m_pairCall == pairCall);
		ESS_ASSERT(!pairCall.IsEmpty());
		ESS_ASSERT(!pairCall.IsEqualIntf(this));

		m_rejectTimer.Stop();
		m_stateTimer.Stop();

		bool isUpdate = (m_state == ST_REDIRECTED);
		
		if (!isUpdate) 
		{
			if(m_log->LogActive(m_infoTag)) 
			{
				*m_log << m_infoTag << "State: Redirected. Pair call: " 
					<< pairCall->UserName() 
					<< ((useRedirectMode) ? " Redirect mode" : " Simple mode")
					<< iLogW::EndRecord;
			}
		
			if(m_owner.Profile().MaxTalkDurationMin() != 0) m_stateTimer.Set(m_owner.Profile().MaxTalkDurationMin() * 60 * 1000);
			
			Stat().IncCurrentTalks();
			Stat().AddWaitAnswerDuration((Utils::TimerTicks::Ticks() - m_beginCall)/ 1000);
			m_beginTalk = Utils::TimerTicks::Ticks();
		}

		m_state = ST_REDIRECTED;

		m_pairCall = pairCall;

		// reset media if necessary
		if (useRedirectMode != IsRedirectMode()) m_media.reset(CreateMediaItem(useRedirectMode)); 

		if (!useRedirectMode) 
		{		
			m_media->SetupPartnerSdp(m_pairCall->SourceSdp());

			if (isUpdate) 
			{
				m_callSip->UpdateMedia();
				return;
			}
		}

		m_callSip->AnswerParticipant();
	}

	// -------------------------------------------------------------------------------

	void CountryFilter::QueuedCall::Release(const QueuedCall *pairCall, int cause)
	{
		ESS_ASSERT(m_pairCall.IsEqualIntf(pairCall));

		if(m_log->LogActive(m_infoTag)) 
		{
			*m_log << m_infoTag << "Release. Cause: " << cause << iLogW::EndRecord;
		}

		m_pairCall.Clear();

		if(!m_callSip.IsEmpty()) m_callSip.Clear()->DestroyParticipant(cause);

		Delete();
	}

	// -------------------------------------------------------------------------------

	void CountryFilter::QueuedCall::Delete()
	{
		if(m_state == ST_DELETE_PROGRESS) return;

		FixEndTalk();

		m_state = ST_DELETE_PROGRESS;
		LogState("Delete");

        m_owner.DeleteCall(*this);
	}

	// -------------------------------------------------------------------------------

	CallMeetingStatList &CountryFilter::QueuedCall::Stat()
	{
		return *m_stat;
	}

	// -------------------------------------------------------------------------------

	void CountryFilter::QueuedCall::FixEndTalk()
	{
		if(m_state != ST_REDIRECTED) return;

		Stat().AddTalkDuration((Utils::TimerTicks::Ticks() - m_beginTalk)/ 1000);
		Stat().DecCurrentTalks();
	}

	// -------------------------------------------------------------------------------

	std::string CountryFilter::QueuedCall::UserName() const
	{
		return m_log->NameStr();
	}

	// -------------------------------------------------------------------------------

	bool CountryFilter::QueuedCall::IsRedirectMode() const
	{
		return m_media->IsRedirectedMode();
	}

	// -------------------------------------------------------------------------------

	boost::shared_ptr<resip::SdpContents> CountryFilter::QueuedCall::SourceSdp()
	{
		return m_sourceSdp;
	}

	// -------------------------------------------------------------------------------

	IMediaItem* CountryFilter::QueuedCall::CreateMediaItem(bool isRedirectMode)
	{
		IMediaItem::OfferProcessor offerProcessor = boost::bind(&T::InitRemoteSdp, this, _1);

		if (isRedirectMode) return new MediaForRedirectMode(*m_log, offerProcessor);
		
		return new MediaForSimpleMode(offerProcessor);
	}

	// -------------------------------------------------------------------------------

	void CountryFilter::QueuedCall::InitRemoteSdp(boost::shared_ptr<resip::SdpContents> offer)
	{
		m_sourceSdp = offer;

		// if (m_state != ST_REDIRECTED) return;

		// cant support reinvite - cicle occurs
		// a  -- Reinvite ->  ipsius  
		// a  <- 200      --  ipsius
		// ------------------ ipsius -- Reinvite ->  b
		// ------------------ ipsius <- 200      --  b
		// a  <- Reinvite --  ipsius  


//		if (m_media->IsRedirectedMode()) return;
			
	// start reinvite
//		m_pairCall->SetPair(this, false);
	}
}



