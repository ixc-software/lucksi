#include "stdafx.h"
#include "iSip/undeferrno.h"

#include "ClientSubscriptionHandler.h"

#include "resip/dum/AppDialog.hxx"
#include <resip/dum/ClientSubscription.hxx>
#include "Utils/ErrorsSubsystem.h"


namespace
{
	iSip::IClientSubscriptionHandler* SafeCast(resip::AppDialog *ptr)
	{
		iSip::IClientSubscriptionHandler *handler =
			dynamic_cast<iSip::IClientSubscriptionHandler*>(ptr);

		ESS_ASSERT(handler != 0);

		return handler;
	}
}

namespace
{
	class ClientSubscriptionHandler : boost::noncopyable,
		public resip::ClientSubscriptionHandler
	{
	// implement of ClientSubscriptionHandler
	private:

		void onUpdatePending(
			resip::ClientSubscriptionHandle h, const resip::SipMessage& msg, bool outOfOrder)
		{
			SafeCast(h->getAppDialog().get())->onUpdatePending(h, msg, outOfOrder);
		}

		// -------------------------------------------------------------------------------------

		void onUpdateActive(
			resip::ClientSubscriptionHandle h, const resip::SipMessage& msg, bool outOfOrder)
		{
			SafeCast(h->getAppDialog().get())->onUpdateActive(h, msg, outOfOrder);
		}

		// -------------------------------------------------------------------------------------

		void onUpdateExtension(
			resip::ClientSubscriptionHandle h, const resip::SipMessage& msg, bool outOfOrder)
		{
			SafeCast(h->getAppDialog().get())->onUpdateExtension(h, msg, outOfOrder);
		}

		// -------------------------------------------------------------------------------------

		void onTerminated(
			resip::ClientSubscriptionHandle h, const resip::SipMessage* msg)
		{
			SafeCast(h->getAppDialog().get())->onTerminated(h, msg);
		}

		// -------------------------------------------------------------------------------------

		void onNewSubscription(
			resip::ClientSubscriptionHandle h, const resip::SipMessage& msg)
		{
			SafeCast(h->getAppDialog().get())->onNewSubscription(h, msg);
		}

		// -------------------------------------------------------------------------------------

		int onRequestRetry(
			resip::ClientSubscriptionHandle h, int retryMinimum, const resip::SipMessage& msg)
		{
			return SafeCast(h->getAppDialog().get())->onRequestRetry(h, retryMinimum, msg);

		}
	};
}


namespace iSip
{
	resip::ClientSubscriptionHandler *CreateClientSubscriptionHandler()
	{
		return new ClientSubscriptionHandler();
	}
}
