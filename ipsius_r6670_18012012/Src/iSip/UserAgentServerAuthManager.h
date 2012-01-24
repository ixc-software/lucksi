#pragma once 


//#include <resip/stack/Auth.hxx>
//#include <resip/stack/Message.hxx>
//#include <resip/dum/UserProfile.hxx>
#include <resip/dum/ServerAuthManager.hxx>
#include "rutil/SharedPtr.hxx"

namespace resip 
{	
	class Auth;
	class DialogUsageManager;
	class SipMessage; 
};

namespace iSip
{

	class ConversationProfile;

	/**
	  This class is used to provide server digest authentication
	  capabilities.  It uses the profile settings in order to determine
	  if a SIP request should be challenged or not, and challenges 
	  appropriately.

	  It is used to challenge auto-answer requests and OOD refer
	  requests.

	  Author: Scott Godin (sgodin AT SipSpectrum DOT com)
	*/

	class UserAgentServerAuthManager: public resip::ServerAuthManager
	{
	public:
		typedef boost::function<boost::shared_ptr<ConversationProfile> (const resip::SipMessage&)> 
			ProfileFinder;

		UserAgentServerAuthManager(resip::DialogUsageManager &dum,
			ProfileFinder profileFinder);

		~UserAgentServerAuthManager();
      
	protected:
		// this call back should async cause a post of UserAuthInfo
		virtual void requestCredential(const resip::Data& user, 
			const resip::Data& realm, 
			const resip::SipMessage& msg,
			const resip::Auth& auth,
			const resip::Data& transactionId );
      
		virtual bool useAuthInt() const;
		virtual bool proxyAuthenticationMode() const;
		virtual const resip::Data& getChallengeRealm(const resip::SipMessage& msg);   
		virtual bool isMyRealm(const resip::Data& realm);
		virtual bool authorizedForThisIdentity(const resip::Data &user, 
			const resip::Data &realm, 
			resip::Uri &fromUri);

		virtual AsyncBool requiresChallenge(const resip::SipMessage& msg);

	private:
		resip::DialogUsageManager &m_dum;

		ProfileFinder m_profileFinder;
	};

}


