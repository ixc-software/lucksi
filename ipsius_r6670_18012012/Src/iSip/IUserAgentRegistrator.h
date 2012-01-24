#pragma once

#include "SipRegistrationState.h"
#include "ConversationProfile.h"
#include "Utils/IBasicInterface.h"
#include "Utils/WeakRef.h"
#include <rutil/SharedPtr.hxx>

namespace iSip
{
	class ISipAccessPointEventsExt;

	class IRegistrationItem : Utils::IBasicInterface
	{
	public:

		typedef boost::function<void (const IRegistrationItem&, SipRegistrationState::Value)> Observer;

		virtual SipRegistrationState::Value GetState() const = 0;

		virtual void RemoveRegistration() = 0;

	};


	class IUserAgentRegistrator : Utils::IBasicInterface
	{
	public:

		typedef IRegistrationItem::Observer Observer;
		
		virtual Utils::WeakRef<IRegistrationItem&>
			AddRegistration(const std::string &accessPointName,
				ISipAccessPointEventsExt &user,
				boost::shared_ptr<ConversationProfile> conversationProfile) = 0;

		virtual void SetDefIncomingCallReceiver(
			const IRegistrationItem &callReceiver) = 0;

	};

}

