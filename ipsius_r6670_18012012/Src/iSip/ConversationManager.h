#pragma once 

#ifdef WIN32
#define BOOST__STDC_CONSTANT_MACROS_DEFINED  // elminate duplicate define warnings under windows
#endif

#include "ISipMedia.h"
#include "ISipAccessPoint.h"
#include "InviteSessionHandler.h"
#include "ConversationProfile.h"
#include "ClientSubscriptionHandler.h"
#include "ServerSubscriptionHandler.h"


#include "ParticipantForkSelectMode.h"
#include "SipUtils.h"

#include "iCore/MsgThread.h"
#include "iCore/MsgObject.h"

#include "iLog/LogWrapper.h"
#include "Utils/ErrorsSubsystem.h"

#include <resip/stack/Uri.hxx>

#include <resip/dum/DialogSetHandler.hxx>
#include <resip/dum/OutOfDialogHandler.hxx>
#include <resip/dum/RedirectHandler.hxx>


namespace resip	{	class DialogUsageManager;	}
namespace iCore {	class MsgThread; }

namespace iSip
{

	class ConversationProfile;
	class RemoteParticipant;
	class MsgRemoteParticipant;
	class SipTrustedNodeList;


	class ConversationManager  : boost::noncopyable,
		public iCore::MsgObject,
		resip::DialogSetHandler,
		resip::OutOfDialogHandler,
		resip::RedirectHandler,
		public ISipAccessPointEvents,
		public IRemoteParticipantCreator

	{
		typedef ConversationManager T;

	public:  
  
		typedef boost::function<boost::shared_ptr<ConversationProfile> (const resip::SipMessage&)> 
			ConversationProfileFinder;

		typedef boost::function<ISipAccessPointEvents* (const ConversationProfile&)> 
			CallReceiver;


		ConversationManager(iCore::MsgThread &thread,
			iLogW::LogSession  &logSession,
			boost::shared_ptr<ConversationProfile> defaultConversationProfile,
			const SipTrustedNodeList &trustedNodeList,
			resip::DialogUsageManager &dum,
			CallReceiver callReceiver,
			ConversationProfileFinder conversationProfileFinder);

		~ConversationManager();

		iLogW::ILogSessionCreator& LogCreator() {	return m_log; }

//		ResipUtils::PrintMode ResipLogMode() const {	return m_resipLogMode(); }

		RemoteParticipant* CreateUACOriginalRemoteParticipant(
			boost::shared_ptr<ConversationProfile>	conversationProfile,
			ParticipantForkSelectMode::Value forkMode);

		boost::shared_ptr<ConversationProfile> 
			GetIncomingConversationProfile(const resip::SipMessage& msg);

		resip::DialogUsageManager& GetDialogUsageManager()	{	return m_dum;	}

		void PostMsg(boost::function<void ()> fn);

		void Shutdown();

	// ISipAccessPointEvents
   public:

		void OnIncomingParticipant(const ISipAccessPoint *src,
			Utils::SafeRef<IRemoteParticipant> newParticipant, 
			const resip::SipMessage &msg, 
			bool autoAnswer, 
			const ConversationProfile &conversationProfile);

		void OnRequestOutgoingParticipant(const ISipAccessPoint *src,
			Utils::SafeRef<IRemoteParticipant> newParticipant, 
			const resip::SipMessage &msg, 
			const ConversationProfile &conversationProfile);

	// IRemoteParticipantCreator 
	public:

		Utils::SafeRef<IRemoteParticipant> CreateOutgoingParticipant(
			Utils::SafeRef<IRemoteParticipantUser> user,
			boost::shared_ptr<Parameters> parameters);
	
	// OutOfDialogHandler
	private:

		void onSuccess(resip::ClientOutOfDialogReqHandle, const resip::SipMessage& response);
		void onFailure(resip::ClientOutOfDialogReqHandle, const resip::SipMessage& response);
		void onReceivedRequest(resip::ServerOutOfDialogReqHandle, const resip::SipMessage& request);

	// RedirectHandler
	private:

		void onRedirectReceived(resip::AppDialogSetHandle, const resip::SipMessage& response);
		bool onTryingNextTarget(resip::AppDialogSetHandle, const resip::SipMessage& request);

	// DialogSetHandler
	private:
		void onTrying(resip::AppDialogSetHandle, const resip::SipMessage &msg);

		void onNonDialogCreatingProvisional(resip::AppDialogSetHandle, const resip::SipMessage &msg);

	private:

		void NewSubscriptionFromRefer(resip::ServerSubscriptionHandle, const resip::SipMessage& sub);

		void RemoteParticipantRegistrator(IRemoteParticipant *item, bool addToList);

		MsgRemoteParticipant* CreateIncomingParticipant(
			Utils::SafeRef<IRemoteParticipant> newParticipant);

		void RequestOutgoingParticipant(
			Utils::SafeRef<IRemoteParticipant> newParticipant, 
			const resip::SipMessage &msg, 
			const iSip::ConversationProfile &conversationProfile);

		void OnDeleteParticipants(IRemoteParticipant *item) {}

		std::string MsgToString(const resip::SipMessage &msg);

	private:
		iLogW::LogSession &m_log;
		iLogW::LogRecordTag m_infoTag;
		iLogW::LogRecordTag m_warningTag;

		boost::shared_ptr<ConversationProfile> m_defaultConversationProfile;

		const SipTrustedNodeList &m_trustedNodeList;

		resip::DialogUsageManager	&m_dum;
		CallReceiver				m_callReceiver;
		ConversationProfileFinder	m_conversationProfileFinder;

		InviteSessionHandler		m_inviteSessionHandler;
		
		boost::scoped_ptr<resip::ClientSubscriptionHandler> m_clientSubscriptionHandler;

		boost::scoped_ptr<ServerSubscriptionHandler> m_serverSubscriptionHandler;

		typedef boost::ptr_list<IRemoteParticipant> ListOfParticipant;
		ListOfParticipant m_participants;
	};

}



