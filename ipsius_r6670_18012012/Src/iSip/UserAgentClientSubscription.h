#pragma once 

#include "IUserAgentSubscription.h"
#include "ClientSubscriptionHandler.h"
#include "Utils/WeakRef.h"

#include <resip/dum/AppDialogSet.hxx>
#include <resip/dum/InviteSessionHandler.hxx>
#include <resip/dum/DialogSetHandler.hxx>
#include <resip/dum/SubscriptionHandler.hxx>

namespace resip
{
	class DialogUsageManager;
	class SipMessage;
	class UserProfile;
}

namespace iLogW {	class ILogSessionCreator; }

namespace iSip
{

	class UserAgentClientSubscription : boost::noncopyable
	{
	public:

		UserAgentClientSubscription(resip::DialogUsageManager &dum);

		~UserAgentClientSubscription();

		void Shutdown();

		IUserAgentSubscription* CreateSubscription(
			iLogW::ILogSessionCreator &logCreator,
			IUserAgentSubscriptionEvent &user,
			resip::SharedPtr<resip::UserProfile> profile,
			const resip::Data &eventType, 
			const resip::NameAddr &target, 
			unsigned int subscriptionTime); 

	// resip::ClientSubscriptionHandler
	private:

		void onUpdatePending(resip::ClientSubscriptionHandle h, const resip::SipMessage& notify, bool outOfOrder);

		void onUpdateActive(resip::ClientSubscriptionHandle h, const resip::SipMessage& notify, bool outOfOrder);

		void onUpdateExtension(resip::ClientSubscriptionHandle, const resip::SipMessage& notify, bool outOfOrder);

		void onTerminated(resip::ClientSubscriptionHandle h, const resip::SipMessage* notify);

		void onNewSubscription(resip::ClientSubscriptionHandle h, const resip::SipMessage& notify);

		int  onRequestRetry(resip::ClientSubscriptionHandle h, int retryMinimum, const resip::SipMessage& notify);

	private:
		resip::DialogUsageManager &m_dum;

		boost::scoped_ptr<resip::ClientSubscriptionHandler> m_clientSubscriptionHandler;

		class ClientSubscription;

		typedef std::vector< Utils::WeakRef<ClientSubscription&> > Subscriptions;

		Subscriptions m_subscriptions;
	};

}

