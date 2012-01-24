#pragma once 

#include "RegistrationHandler.h"
#include "SipRegistrationState.h"
#include "IUserAgentRegistrator.h"
#include "SipUtils.h"

#include "Utils/IBasicInterface.h"
#include "Utils/WeakRef.h"
#include "Utils/SafeRef.h"
#include "Utils/ManagedList.h"

#include "iLog/LogWrapper.h"



namespace resip
{
	class DialogUsageManager;
	class SipMessage;
}

namespace iSip
{
	class ISipAccessPointEvents;
	class ConversationProfile;

	class UserAgentRegistrator : boost::noncopyable,
		public IUserAgentRegistrator
	{
		typedef UserAgentRegistrator T;

		class Item;

	public:  

		UserAgentRegistrator(
			resip::DialogUsageManager &dum,
			iLogW::LogSession  &logSession,
			boost::shared_ptr<ConversationProfile> defaultConversationProfile);

		~UserAgentRegistrator();

		void Shutdown();

		boost::shared_ptr<ConversationProfile> 
			GetIncomingConversationProfile(const resip::SipMessage& msg) const;

		ISipAccessPointEvents* 
			CallsReceiver(const ConversationProfile &conversationProfile);

	// IUserAgentRegistrator
	public:

		Utils::WeakRef<IRegistrationItem&>
			AddRegistration(const std::string &accessPointName,
				ISipAccessPointEventsExt &user,
				boost::shared_ptr<ConversationProfile> conversationProfile);

		void SetDefIncomingCallReceiver(const IRegistrationItem &callReceiver);

	private:

		Item* FindRegistrationItem(const ConversationProfile &conversationProfile);

		boost::shared_ptr<ConversationProfile> 
			DefIncomingConversationProfile() const;

		void PointRegistrator(Item *, bool addToList);

	private:

		resip::DialogUsageManager &m_dum;
		RegistrationHandler m_registrationHandler;

		iLogW::LogSession &m_log;
		iLogW::LogRecordTag m_infoTag;

		boost::shared_ptr<ConversationProfile> m_defaultConversationProfile;

		typedef boost::ptr_vector<Item> ListItem;
		ListItem m_items;

		Utils::WeakRef<Item&> m_defcallReceiver;
	};

}



