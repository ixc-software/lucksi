#pragma once 

#include "stdafx.h"

#include "IRemoteParticipant.h"
#include "ISipMediaLine.h"

#include "iLog/LogWrapper.h"
#include "Utils/SafeRef.h"
#include "iCore/MsgTimer.h"
#include "iCore/MsgThread.h"
#include "iCore/MsgObject.h"


namespace sdpcontainer	
{	
	class Sdp;
	class SdpMediaLine; 
}

namespace iSip
{
	class ISipAccessPoint;
	class MediaLineList;

	class AutotrainerSipCall : boost::noncopyable,
		public iCore::MsgObject,
		public IRemoteParticipantUser
	{

		typedef AutotrainerSipCall T;
		
	public:

		typedef boost::function<void (T *)> Command;

		typedef std::list<Command> Commands;

		AutotrainerSipCall(iCore::MsgThread &thread,
			iLogW::ILogSessionCreator &logCreator,
			boost::function<void (T *)> deleter,
			const Commands &commands);

		AutotrainerSipCall(iCore::MsgThread &thread,
			iLogW::ILogSessionCreator &logCreator,
			boost::function<void (T *)> deleter,
			Utils::SafeRef<IRemoteParticipant> remoteParticipant,
			const Commands &commands);
		
		~AutotrainerSipCall();

	public:

		void StartOutgoingCall(
			Utils::SafeRef<IRemoteParticipantCreator> callCreator,
			const resip::Uri &destinationUri);

		void RedirectTo(const resip::Uri &destinationUri);

		void Wait(int timeout);

		void Alerting();

		void Answer();

		void Hold();

		void Unhold();

		void Close(int statusCode = 0);

	// IRemoteParticipantUser
	private:
		
		std::string CallUserName() const;

		ISipMedia* Media();

		const ISipMedia* Media() const;

		boost::shared_ptr<iReg::CallRecordWrapper> CallRecordByUser();

		void OnParticipantTerminated(const IRemoteParticipant *src, 
			unsigned int statusCode);
		
		void OnRefer(const IRemoteParticipant *src, const resip::SipMessage &msg);

		void OnParticipantAlerting(const IRemoteParticipant *src, 
			const resip::SipMessage& msg);

		void OnParticipantConnected(const IRemoteParticipant *src,
			const resip::SipMessage &msg);

		void OnParticipantRedirectSuccess(const IRemoteParticipant *src);

		void OnParticipantRedirectFailure(const IRemoteParticipant *src, 
			unsigned int statusCode);

		void OnForkingOccured(const IRemoteParticipant *origCall, 
			Utils::SafeRef<IRemoteParticipant> newCall,
			const resip::SipMessage &msg);

	private:

		void InitRemoteSdp(boost::shared_ptr<resip::SdpContents> offer);

	private:

		void OnCommand();

		void OnTimeout(iCore::MsgTimer *pT);

		void CreateMedia();

		void DeleteCall();

	private:

		iCore::MsgTimer m_timer;

		boost::scoped_ptr<iLogW::LogSession>  m_log;
		iLogW::LogRecordTag m_infoTag;
		boost::function<void (T *)> m_deleter;

		bool m_callDeleted;

		Commands m_commands;
		
		Utils::SafeRef<IRemoteParticipant> m_remoteParticipant;

		boost::scoped_ptr<MediaLineList> m_media;

		boost::scoped_ptr<ISipMediaLineEvents> m_mediaLine;
	};
}


