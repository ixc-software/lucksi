#pragma once

#include "ConversationManager.h"
#include "ConversationProfile.h"
#include "ParticipantForkSelectMode.h"
#include "InviteSessionHandler.h"
#include "ClientSubscriptionHandler.h"
#include "IRemoteParticipant.h"

#include "iLog/LogWrapper.h"

#include "Utils/WeakRef.h"
#include "Utils/SafeRef.h"

#include <resip/stack/SdpContents.hxx>
#include <resip/dum/AppDialogSet.hxx>
#include <resip/dum/AppDialog.hxx>
#include <resip/dum/InviteSessionHandler.hxx>
#include <resip/dum/SubscriptionHandler.hxx>

namespace iReg 
{	
	class CallRecordWrapper;	 
	class CallSideRecord;
}

namespace resip
{
	class DialogUsageManager;
	class SipMessage;
}

namespace sdpcontainer
{
	class Sdp; 
	class SdpMediaLine;
}

namespace iSip
{
	
	class ConversationManager;
	class RemoteParticipantDialogSet;

	/**
	  This class represent a rem_conversationProfilemote participant.  
	  A remote participant is a participant with a network connection 
	  to a remote entity. This implementation is for a SIP / RTP connections.
	*/

	class RemoteParticipant : boost::noncopyable,
		public virtual Utils::SafeRefServer,
		public resip::AppDialog,
		public IInviteSessionEvent,
		public IClientSubscriptionHandler,
		public IRemoteParticipant
	{
		typedef RemoteParticipant T;

		enum State
		{
			Connecting=1, 
			Accepted,
			Connected,
			Redirecting,
			Reinviting,
			Replacing,
			PendingOODRefer,
			Terminating
		};

	public:
		// UAC
		RemoteParticipant(ConversationManager &conversationManager, 
			resip::DialogUsageManager &dum,
			RemoteParticipantDialogSet &remoteParticipantDialogSet,
			iLogW::ILogSessionCreator &logCreator,
			boost::function<void (RemoteParticipant*, bool)> registrator,
			boost::shared_ptr<ConversationProfile> conversationProfile,
			Utils::SafeRef<IRemoteParticipantUser> user,
			const resip::NameAddr &destination);  

		// UAS or forked leg
		RemoteParticipant(ConversationManager &conversationManager,
			resip::DialogUsageManager &dum,
			RemoteParticipantDialogSet &remoteParticipantDialogSet,
			iLogW::ILogSessionCreator &logCreator,
			boost::function<void (RemoteParticipant*, bool)> registrator,
			boost::shared_ptr<ConversationProfile> conversationProfile);

		~RemoteParticipant();

		// std::string Name() const;

		Utils::WeakRef<RemoteParticipant&> SelfRef() 
		{	
			return m_selfHost.Create<RemoteParticipant&>(*this); 
		}

		const resip::DialogId& DialogId() const {	return m_dialogId; }

		void destroyParticipant();

		void OnNonDialogCreatingProvisional(const resip::SipMessage& msg); 

		void OnForkingOccured(const IRemoteParticipant *src, 
			Utils::SafeRef<IRemoteParticipant> newCall,
			const resip::SipMessage &msg);

		void OnReplacedBy(Utils::SafeRef<IRemoteParticipant> newCall);

		void doReferNoSub(const resip::SipMessage& msg);
		void setPendingOODReferInfo(resip::ServerOutOfDialogReqHandle ood, const resip::SipMessage& referMsg); // OOD-Refer (no Sub)
		void setPendingOODReferInfo(resip::ServerSubscriptionHandle ss, const resip::SipMessage& referMsg); // OOD-Refer (with Sub)

	// IInviteSessionEvent
	private:

		void onNewSession(resip::ClientInviteSessionHandle h, resip::InviteSession::OfferAnswerType oat, const resip::SipMessage& msg);
		void onNewSession(resip::ServerInviteSessionHandle h, resip::InviteSession::OfferAnswerType oat, const resip::SipMessage& msg);
		void onFailure(resip::ClientInviteSessionHandle h, const resip::SipMessage& msg);
		void onEarlyMedia(resip::ClientInviteSessionHandle, const resip::SipMessage&, const resip::SdpContents&);
		void onProvisional(resip::ClientInviteSessionHandle, const resip::SipMessage& msg);
		void onConnected(resip::ClientInviteSessionHandle h, const resip::SipMessage& msg);
		void onConnected(resip::InviteSessionHandle, const resip::SipMessage& msg);
		void onConnectedConfirmed(resip::InviteSessionHandle, const resip::SipMessage &msg);
		void onStaleCallTimeout(resip::ClientInviteSessionHandle);
		void onTerminated(resip::InviteSessionHandle h, resip::InviteSessionHandler::TerminatedReason reason, const resip::SipMessage* msg);
		void onRedirected(resip::ClientInviteSessionHandle, const resip::SipMessage& msg);
		void onAnswer(resip::InviteSessionHandle, const resip::SipMessage& msg, const resip::SdpContents&);
		void onOffer(resip::InviteSessionHandle handle, const resip::SipMessage& msg, const resip::SdpContents& offer);
		void onOfferRequired(resip::InviteSessionHandle, const resip::SipMessage& msg);
		void onOfferRejected(resip::InviteSessionHandle, const resip::SipMessage* msg);
		void onOfferRequestRejected(resip::InviteSessionHandle, const resip::SipMessage& msg);
		void onRemoteSdpChanged(resip::InviteSessionHandle, const resip::SipMessage& msg, const resip::SdpContents& sdp);
		void onInfo(resip::InviteSessionHandle, const resip::SipMessage& msg);
		void onInfoSuccess(resip::InviteSessionHandle, const resip::SipMessage& msg);
		void onInfoFailure(resip::InviteSessionHandle, const resip::SipMessage& msg);
		void onRefer(resip::InviteSessionHandle, resip::ServerSubscriptionHandle, const resip::SipMessage& msg);
		void onReferAccepted(resip::InviteSessionHandle, resip::ClientSubscriptionHandle, const resip::SipMessage& msg);
		void onReferRejected(resip::InviteSessionHandle, const resip::SipMessage& msg);
		void onReferNoSub(resip::InviteSessionHandle, const resip::SipMessage& msg);
		void onMessage(resip::InviteSessionHandle, const resip::SipMessage& msg);
		void onMessageSuccess(resip::InviteSessionHandle, const resip::SipMessage& msg);
		void onMessageFailure(resip::InviteSessionHandle, const resip::SipMessage& msg);
		void onForkDestroyed(resip::ClientInviteSessionHandle);
	
	// IClientSubscriptionHandler
	private:
		void onUpdatePending(resip::ClientSubscriptionHandle h, const resip::SipMessage& notify, bool outOfOrder);
		void onUpdateActive(resip::ClientSubscriptionHandle h, const resip::SipMessage& notify, bool outOfOrder);
		void onUpdateExtension(resip::ClientSubscriptionHandle, const resip::SipMessage& notify, bool outOfOrder);
		void onTerminated(resip::ClientSubscriptionHandle h, const resip::SipMessage* notify);
		void onNewSubscription(resip::ClientSubscriptionHandle h, const resip::SipMessage& notify);
		int  onRequestRetry(resip::ClientSubscriptionHandle h, int retryMinimum, const resip::SipMessage& notify);

	// IRemoteParticipant
	public:       

		std::string NameParticipant() const;

		void LinkUser(Utils::SafeRef<IRemoteParticipantUser> user);

		void RelatedParticipants(RelatedParticipantList &list);

		boost::shared_ptr<ConversationProfile> GetConversationProfile() const;

		void MediaObjectIsReady();

		boost::shared_ptr<iReg::CallRecordWrapper> CallRecord();

		void AlertParticipant(bool earlyFlag = true);

		void AnswerParticipant();

		void UpdateMedia();

		void DestroyParticipant(unsigned int rejectCode, //  = 480 - Temporarily Unavailable
			const std::string &description); 

		void RedirectParticipant(const resip::NameAddr &destination);

		void RedirectToParticipant(IRemoteParticipant &dst);

	private:       
		
		void acceptPendingOODRefer();
		void rejectPendingOODRefer(unsigned int statusCode);
		void redirectPendingOODRefer(const resip::NameAddr& destination);
		void processReferNotify(const resip::SipMessage& notify);

	private:       
		
		void Refer(const resip::SipMessage &msg,
			resip::ServerSubscriptionHandle ss = resip::ServerSubscriptionHandle());

		void RedirectToParticipantImpl(resip::InviteSessionHandle destHandle);

		void provideOffer(bool postOfferAccept);
		bool ProvideAnswer(bool postAnswerAccept, bool postAnswerAlert);

		bool formMidDialogSdpOfferOrAnswer(const resip::SdpContents& localSdp, const resip::SdpContents& remoteSdp, resip::SdpContents& newSdp, bool offer);

		void InitiateRemoteCall(const resip::NameAddr& destination);

		resip::InviteSessionHandle getInviteSessionHandle() { return m_inviteSessionHandle; }

		void doProvideOfferAnswer(bool offer, 
			const resip::SdpContents &sdp, 
			bool postOfferAnswerAccept, 
			bool postAnswerAlert);

		static std::string StateToString(State state);
		void stateTransition(State state);

		void InitRemoteSdp(const resip::SdpContents &offer);

		std::string MsgToString(const resip::SipMessage &msg);

	private:
		Utils::WeakRefHost m_selfHost;

		boost::scoped_ptr<iLogW::LogSession>  m_log;
		iLogW::LogRecordTag m_infoTag;
		iLogW::LogRecordTag m_warningTag;
		iLogW::LogRecordTag m_debugTag;

		boost::function<void (RemoteParticipant*, bool)> m_registrator;


		ConversationManager        &m_conversationManager;
		resip::DialogUsageManager  &m_dum;
		resip::InviteSessionHandle m_inviteSessionHandle; 
		RemoteParticipantDialogSet &m_dialogSet;   

		Utils::SafeRef<IRemoteParticipantUser> m_user;
		boost::shared_ptr<ConversationProfile> m_conversationProfile;
		boost::shared_ptr<iReg::CallRecordWrapper> m_callRecord;
		iReg::CallSideRecord &m_regRecord;

		resip::DialogId m_dialogId;

		State m_state;
		bool  m_offerRequired;

		resip::SipMessage mPendingOODReferMsg;
		resip::ServerOutOfDialogReqHandle mPendingOODReferNoSubHandle;
		resip::ServerSubscriptionHandle mPendingOODReferSubHandle;

		boost::function<void ()> m_pendingRequest;

	};
}

