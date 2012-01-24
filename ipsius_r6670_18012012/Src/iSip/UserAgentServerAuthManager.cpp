#include "stdafx.h"
#include "iSip/undeferrno.h"

#include "UserAgentServerAuthManager.h"
#include "ConversationProfile.h"
#include <resip/dum/DialogUsageManager.hxx>
#include <resip/dum/ServerAuthManager.hxx>
#include <resip/dum/UserAuthInfo.hxx>
#include <rutil/MD5Stream.hxx>
#include <rutil/WinLeakCheck.hxx>
#include "Utils/ErrorsSubsystem.h"


namespace iSip
{

	UserAgentServerAuthManager::UserAgentServerAuthManager(resip::DialogUsageManager &dum,
		ProfileFinder profileFinder) :
		ServerAuthManager(dum, dum.dumIncomingTarget()),
		m_dum(dum),
		m_profileFinder(profileFinder)
	{
	}

	// -------------------------------------------------------------------------------------
	
	UserAgentServerAuthManager::~UserAgentServerAuthManager()
	{
	}

	// -------------------------------------------------------------------------------------bool

	bool UserAgentServerAuthManager::useAuthInt() const
	{
		return true;
	}

	// -------------------------------------------------------------------------------------

	bool UserAgentServerAuthManager::proxyAuthenticationMode() const
	{
		return false;  // Challenge with 401
	}

	// -------------------------------------------------------------------------------------

	const resip::Data&
	UserAgentServerAuthManager::getChallengeRealm(const resip::SipMessage& msg)
	{
		return m_profileFinder(msg)->UserProfile().getDefaultFrom().uri().host();
	}

	// -------------------------------------------------------------------------------------

	bool UserAgentServerAuthManager::isMyRealm(const resip::Data& realm)
	{
		return true;  // .slg. this means we will try to find credentials for any authorization headers
					 // could improve this by looking through all active conversation profiles to see if realm exists
	}

	// -------------------------------------------------------------------------------------bool

	bool UserAgentServerAuthManager::authorizedForThisIdentity(const resip::Data &user,
		const resip::Data &realm,
		resip::Uri &fromUri)
	{
		return true;  // We don't care who the request came from
	}

	// -------------------------------------------------------------------------------------

	resip::ServerAuthManager::AsyncBool
		UserAgentServerAuthManager::requiresChallenge(const resip::SipMessage& msg)
	{
		ESS_ASSERT(msg.isRequest());

		ConversationProfile *profile = m_profileFinder(msg).get();

		switch(msg.method())
		{
			case resip::REFER:

				// We want to challenge OOD Refer requests and Invite Requests with Auto-Answer indications
				if (profile->GetSecureChallengeProfile().ChallengeOODReferRequests()) break;

				if (!msg.header(resip::h_To).exists(resip::p_tag)) break;

				// Don't challenge OOD Refer requests have a valid TargetDialog header
				if (msg.exists(resip::h_TargetDialog)) break;

				if (m_dum.findInviteSession(msg.header(resip::h_TargetDialog)).first == resip::InviteSessionHandle::NotValid())
				{
					return True;
				}

			break;

			case resip::INVITE:
				if (profile->GetSecureChallengeProfile().ChallengeAutoAnswerRequests() &&
					profile->GetAutoAnswerProfile().ShouldAutoAnswer(msg))
				{
					return True;
				}
			break;
		}

		// Default to not challenge
		return False;
	}

	// -------------------------------------------------------------------------------------

	void UserAgentServerAuthManager::requestCredential(const resip::Data& user,
		const resip::Data& realm,
		const resip::SipMessage& msg,
		const resip::Auth& auth,
		const resip::Data& transactionId )
	{
		const resip::UserProfile::DigestCredential& digestCredential =
			m_profileFinder(msg)->UserProfile().getDigestCredential(realm);

		resip::MD5Stream a1;
		a1 << digestCredential.user
			<< resip::Symbols::COLON
			<< digestCredential.realm
			<< resip::Symbols::COLON
			<< digestCredential.password;
		a1.flush();

		resip::UserAuthInfo* userAuthInfo =
			new resip::UserAuthInfo(user,realm,a1.getHex(),transactionId);

		m_dum.post( userAuthInfo );
	}
 }
