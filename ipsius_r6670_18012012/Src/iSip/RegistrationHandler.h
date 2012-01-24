#pragma once 

#include <resip/dum/RegistrationHandler.hxx>

namespace resip
{
	class DialogUsageManager;
	class SipMessage;
}

namespace iSip
{
	class RegistrationHandler : boost::noncopyable,
		resip::ClientRegistrationHandler
	{
	public:  
		RegistrationHandler(resip::DialogUsageManager &dum);

	// resip::ClientRegistrationHandler
	private:

		void onSuccess(resip::ClientRegistrationHandle h, const resip::SipMessage& response);

		void onFailure(resip::ClientRegistrationHandle h, const resip::SipMessage& response);

		void onRemoved(resip::ClientRegistrationHandle h, const resip::SipMessage& response);

		int onRequestRetry(resip::ClientRegistrationHandle h, int retryMinimum, const resip::SipMessage& msg);
	};
}





