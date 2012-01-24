#pragma once

#include "ConversationProfile.h"
#include "Utils/IBasicInterface.h"
#include <resip/stack/NameAddr.hxx>

namespace iSip
{

	class IUserAgentSubscription : Utils::IBasicInterface
	{
	public:

	};

	// -------------------------------------------------------------------------------------

	class IUserAgentSubscriptionEvent : Utils::IBasicInterface
	{
	public:
		/**
		 Callback used when a subscription has received an event notification 
		 from the server.

		 @param handle Subscription handle that event applies to
		 @param notifyData Data representation of the event received
		*/
		virtual void SubscriptionNotify(const IUserAgentSubscription &src, const resip::Data& notifyData) = 0; 

		/**
		 Callback used when a subscription is terminated.

		 @param statusCode The status code that caused termination.  If
						   application terminated, then statusCode will be 0.
		*/
		virtual void SubscriptionTerminated(const IUserAgentSubscription &src, unsigned int statusCode) = 0;
	};


}

