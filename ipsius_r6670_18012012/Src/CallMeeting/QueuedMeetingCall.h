#pragma once 

#include "stdafx.h"
#include "Sip/undeferrno.h"
#include "iLog/LogWrapper.h"
#include "CountryFilter.h"
#include "Utils/TimerTicks.h"
#include "Utils/Random.h"
#include "iSip/IRemoteParticipant.h"

namespace resip {	class SdpContents;	}

namespace CallMeeting
{
	class CallMeetingStatList;

	class IMediaItem;

	class CountryFilter::QueuedCall : boost::noncopyable,
		public Utils::SafeRefServer,
		public iSip::IRemoteParticipantUser
	{
		typedef QueuedCall T;

	public:

		QueuedCall(CountryFilter &owner,
			iLogW::ILogSessionCreator &logCreator,
			boost::shared_ptr<CallMeetingStatList> stat,
            Utils::SafeRef<iSip::IRemoteParticipant> callSip,
			const std::string &calledAddr);

		~QueuedCall();

		const std::string &CalledAddr() const;

		void Process();
		
		bool IsActive() const;

		bool IsReadyForOtherFilter() const;

		static void redirectToParticipant(QueuedCall &call1, QueuedCall &call2, bool useSipRedirect);

		// IRemoteParticipantUser
	private:

		std::string CallUserName() const;

		iSip::ISipMedia* Media();

		const iSip::ISipMedia* Media() const;

		boost::shared_ptr<iReg::CallRecordWrapper> CallRecordByUser();

		void OnParticipantTerminated(const iSip::IRemoteParticipant *src, 
			unsigned int statusCode);

		void OnRefer(const iSip::IRemoteParticipant *src, const resip::SipMessage &msg);

		void OnParticipantAlerting(const iSip::IRemoteParticipant *src, 
			const resip::SipMessage& msg);

		void OnParticipantConnected(const iSip::IRemoteParticipant *src,
			const resip::SipMessage &msg);

		void OnParticipantRedirectSuccess(const iSip::IRemoteParticipant *src);

		void OnParticipantRedirectFailure(const iSip::IRemoteParticipant *src, 
			unsigned int statusCode);

		void OnForkingOccured(const iSip::IRemoteParticipant *origCall, 
			Utils::SafeRef<iSip::IRemoteParticipant> newCall,
			const resip::SipMessage &msg);

	private:
		void Trying();
		void Alerting();
		void Ready();
		void ReadyForOtherFilter();
		void Reject(const std::string &cause);
		void LogState(const std::string &state);

	private:
		
		void SetPair(Utils::SafeRef<QueuedCall> pairCall, bool useRedirectMode);


		void Release(const QueuedCall *, int cause = 0);
		void Delete();
		CallMeetingStatList &Stat();
		void FixEndTalk();

		std::string UserName() const;

		bool IsRedirectMode() const;

		boost::shared_ptr<resip::SdpContents> SourceSdp();

		IMediaItem* CreateMediaItem(bool isRedirectMode);

		void InitRemoteSdp(boost::shared_ptr<resip::SdpContents> offer);

	private:

		CountryFilter &m_owner;
		Utils::Random m_random;

		boost::scoped_ptr<iLogW::LogSession>  m_log;
		iLogW::LogRecordTag m_infoTag;

		enum State
		{
			ST_TRYING,
			ST_ALERTING,
			ST_READY,
			ST_READY_OTHER_FILTER,
			ST_REDIRECTED,
			ST_DELETE_PROGRESS
		};

		State m_state;
		Utils::TimerTicks m_rejectTimer;
		Utils::TimerTicks m_stateTimer;

		Platform::dword m_beginCall;
		Platform::dword m_beginTalk;
		boost::shared_ptr<CallMeetingStatList> m_stat;

		Utils::SafeRef<iSip::IRemoteParticipant> m_callSip;
		std::string m_calledAddr;
		
		boost::scoped_ptr<IMediaItem> m_media;

		boost::shared_ptr<resip::SdpContents> m_sourceSdp;

		Utils::SafeRef<QueuedCall> m_pairCall;
	};
};



