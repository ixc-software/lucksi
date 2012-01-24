#pragma once

#include "IRemoteParticipant.h"
#include "ISipMedia.h"
#include "Sdp/Sdp.h"
#include "ConversationProfile.h"
#include "Utils/WeakRef.h"
#include "iLog/LogWrapper.h"

#include <resip/dum/AppDialogSet.hxx>
#include <resip/dum/AppDialog.hxx>
#include <resip/dum/InviteSessionHandler.hxx>
#include <resip/dum/DialogSetHandler.hxx>
#include <resip/dum/SubscriptionHandler.hxx>


namespace resip {	class DialogUsageManager; }

namespace iSip
{
	class ConversationManager;
	class RemoteParticipant;

	class RemoteParticipantDialogSet : boost::noncopyable,
		public resip::AppDialogSet,
		public resip::DialogSetHandler,
		public IRemoteParticipantCreator
	{
		typedef RemoteParticipantDialogSet T;

	public:

		RemoteParticipantDialogSet(resip::DialogUsageManager &dum,
			ConversationManager& conversationManager,
			iLogW::ILogSessionCreator &logCreator,
			ParticipantForkSelectMode::Value forkSelectMode = ParticipantForkSelectMode::Automatic);

		~RemoteParticipantDialogSet();

	// IRemoteParticipantCreator 
	public:

		Utils::SafeRef<IRemoteParticipant> CreateOutgoingParticipant(
			Utils::SafeRef<IRemoteParticipantUser> user,
			boost::shared_ptr<IRemoteParticipantCreator::Parameters> parameters);
	
	public:

		RemoteParticipant* CreateUACOriginalRemoteParticipant(
			boost::shared_ptr<ConversationProfile>	conversationProfile);

		void SetUACConnected(const resip::DialogId& dialogId, 
			Utils::WeakRef<RemoteParticipant&> ref);

		bool isUACConnected()  const
		{
			return !m_connectedRemoteParticipant.Empty();
		}
		
		bool isStaleFork(const RemoteParticipant &participant)  const
		{
			return !m_connectedRemoteParticipant.Empty() &&
				&m_connectedRemoteParticipant.Value() != &participant;
		}

		ParticipantForkSelectMode::Value getForkSelectMode()  const
		{
			return mForkSelectMode;
		}

		bool IsActiveRemoteParticipant(const IRemoteParticipant &val)  const;
		
		void SetActiveRemoteParticipant(Utils::WeakRef<RemoteParticipant&> activeRemoteParticipant);

		RemoteParticipant *CreateNewParticipantForRedirected(const RemoteParticipant &redirectedParticipant);


	// resip::DialogSetHandler
	private:

		void onTrying(resip::AppDialogSetHandle, const resip::SipMessage& msg);
		void onNonDialogCreatingProvisional(resip::AppDialogSetHandle, const resip::SipMessage& msg);

	// resip::AppDialogSet
	private:

		resip::SharedPtr<resip::UserProfile> selectUASUserProfile(const resip::SipMessage &msg); 
		resip::AppDialog* createAppDialog(const resip::SipMessage &msg);

	private:

		std::string MsgToString(const resip::SipMessage &msg);

		void RemoteParticipantRegistrator(RemoteParticipant *item, bool addToList);

		RemoteParticipant* CreateUasOrForkParticipant();

	private:

		boost::scoped_ptr<iLogW::LogSession>  m_log;
		iLogW::LogRecordTag m_infoTag;
		iLogW::LogRecordTag m_warningTag;

		ConversationManager &m_conversationManager;   
		
		boost::shared_ptr<ConversationProfile>	  m_conversationProfile;

		ParticipantForkSelectMode::Value mForkSelectMode;

		typedef std::vector<RemoteParticipant*> Dialogs;
		Dialogs m_dialogs;

		// for memory management, store outgoing participant before 
		// move ownership to dum
		std::auto_ptr<RemoteParticipant> m_uacOriginalRemoteParticipant;

		Utils::WeakRef<RemoteParticipant&> m_activeRemoteParticipant;
		Utils::WeakRef<RemoteParticipant&> m_connectedRemoteParticipant;

	};
}


