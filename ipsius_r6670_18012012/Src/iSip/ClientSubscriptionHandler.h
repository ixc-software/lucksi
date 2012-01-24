#pragma once 

#include "stdafx.h"

#include "Utils/IBasicInterface.h"

#include <resip/dum/SubscriptionHandler.hxx>

namespace iSip
{

	class IClientSubscriptionHandler : public Utils::IBasicInterface
	{
	public:

		virtual void onUpdatePending(
			resip::ClientSubscriptionHandle h, const resip::SipMessage& notify, bool outOfOrder) = 0;

		virtual void onUpdateActive(
			resip::ClientSubscriptionHandle h, const resip::SipMessage& notify, bool outOfOrder) = 0;

		virtual void onUpdateExtension(
			resip::ClientSubscriptionHandle, const resip::SipMessage& notify, bool outOfOrder) = 0;

		virtual void onTerminated(
			resip::ClientSubscriptionHandle h, const resip::SipMessage* notify) = 0;

		virtual void onNewSubscription(
			resip::ClientSubscriptionHandle h, const resip::SipMessage& notify) = 0;

		virtual int  onRequestRetry(
			resip::ClientSubscriptionHandle h, int retryMinimum, const resip::SipMessage& notify) = 0;

	};


	resip::ClientSubscriptionHandler* CreateClientSubscriptionHandler();

};


