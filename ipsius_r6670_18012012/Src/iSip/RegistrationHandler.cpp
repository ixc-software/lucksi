#include "stdafx.h"
#include "iSip/undeferrno.h"
#include "RegistrationHandler.h"

#include "Utils/ErrorsSubsystem.h"

#include <resip/dum/DialogUsageManager.hxx>
#include <resip/dum/AppDialogSet.hxx>
#include <resip/dum/ClientRegistration.hxx>

namespace {

	resip::ClientRegistrationHandler& SafeCast(resip::ClientRegistrationHandle h)
	{
		resip::ClientRegistrationHandler *item =
			dynamic_cast<resip::ClientRegistrationHandler*>(h->getAppDialogSet().get());

		ESS_ASSERT(item != 0);

		return *item;
	}
}

namespace iSip
{

	RegistrationHandler::RegistrationHandler(resip::DialogUsageManager &dum)
	{
		dum.setClientRegistrationHandler(this);
	}

	// -------------------------------------------------------------------------------------
	// Registration Handler

	void RegistrationHandler::onSuccess(resip::ClientRegistrationHandle h, const resip::SipMessage &response)
	{
		SafeCast(h).onSuccess(h, response);
	}

	// -------------------------------------------------------------------------------------

	void RegistrationHandler::onFailure(resip::ClientRegistrationHandle h, const resip::SipMessage& response)
	{
		SafeCast(h).onFailure(h, response);
	}

	// -------------------------------------------------------------------------------------

	void RegistrationHandler::onRemoved(resip::ClientRegistrationHandle h, const resip::SipMessage& response)
	{
		SafeCast(h).onRemoved(h, response);
	}

	// -------------------------------------------------------------------------------------

	int RegistrationHandler::onRequestRetry(resip::ClientRegistrationHandle h, int retryMinimum, const resip::SipMessage& msg)
	{
		return SafeCast(h).onRequestRetry(h, retryMinimum, msg);
	}
}





