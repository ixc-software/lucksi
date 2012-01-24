#include "stdafx.h"
#include "iSip/undeferrno.h"
#include "ConversationProfile.h"

#include <resip/dum/UserProfile.hxx>
#include <resip/stack/SipMessage.hxx>
#include <resip/stack/ExtensionParameter.hxx>
#include "Utils/ErrorsSubsystem.h"


namespace
{
	static const resip::ExtensionParameter p_answerafter("answer-after");
	static const resip::ExtensionParameter p_required("required");
}


namespace iSip
{

	RegistrationProfile::RegistrationProfile( resip::UserProfile &userProfile ) :
		m_userProfile(userProfile)
	{
		m_enableRegistration = true;

		m_registrationTime = m_userProfile.getDefaultRegistrationTime();
		m_registrationRetryTime = 30;
		m_userProfile.setDefaultRegistrationRetryTime(m_registrationRetryTime);
	}

	// -------------------------------------------------------------------------------------

	RegistrationProfile::RegistrationProfile(resip::UserProfile &userProfile,
		const RegistrationProfile &src) :
		m_userProfile(userProfile)
	{
		m_enableRegistration = src.m_enableRegistration;

		m_registrationTime = src.m_registrationTime;
		m_registrationRetryTime = src.m_registrationRetryTime;

		m_userProfile.setDefaultRegistrationRetryTime(m_registrationRetryTime);
	}

	// -------------------------------------------------------------------------------------

	void RegistrationProfile::EnableRegistration( bool val )
	{
		m_enableRegistration = val;

		if (m_enableRegistration)
		{
			ESS_ASSERT (m_registrationTime != 0);
			m_userProfile.setDefaultRegistrationTime(m_registrationTime);
			m_userProfile.setDefaultRegistrationRetryTime(m_registrationRetryTime);
		}
		else
		{
			m_userProfile.setDefaultRegistrationTime(0);
			m_userProfile.setDefaultRegistrationRetryTime(0);
		}
	}

	// -------------------------------------------------------------------------------------

	bool RegistrationProfile::EnableRegistration() const
	{
		return m_enableRegistration;
	}

	// -------------------------------------------------------------------------------------

	void RegistrationProfile::RegistrationTimeSec( int val )
	{
		m_registrationTime = val;

		EnableRegistration(val != 0);
	}

	// -------------------------------------------------------------------------------------

	int RegistrationProfile::RegistrationTimeSec() const
	{
		return m_registrationTime;
	}

	// -------------------------------------------------------------------------------------

	void RegistrationProfile::RegistrationRetryTimeSec( int val )
	{
		m_registrationRetryTime = val;

		EnableRegistration(m_enableRegistration);
	}

	// -------------------------------------------------------------------------------------

	int RegistrationProfile::RegistrationRetryTimeSec() const
	{
		return m_registrationRetryTime;
	}
}

// -------------------------------------------------------------------------------------

namespace iSip
{

	bool AutoAnswerProfile::ShouldAutoAnswer(const resip::SipMessage &inviteRequest, bool *required) const
	{
		ESS_ASSERT(inviteRequest.method() == resip::INVITE);

		bool shouldAutoAnswer = false;
		bool autoAnswerRequired = false;

		if (inviteRequest.exists(resip::h_PrivAnswerMode) && inviteRequest.header(resip::h_PrivAnswerMode).value() == "Auto")
		{
			if (m_allowPriorityAutoAnswer) shouldAutoAnswer = true;

			if (inviteRequest.header(resip::h_PrivAnswerMode).exists(p_required)) autoAnswerRequired = true;

		}
		else if (inviteRequest.exists(resip::h_AnswerMode) && inviteRequest.header(resip::h_AnswerMode).value() == "Auto")
		{
			if (m_allowAutoAnswer) shouldAutoAnswer = true;

			if (inviteRequest.header(resip::h_AnswerMode).exists(p_required)) autoAnswerRequired = true;
		}
		else if (m_allowAutoAnswer && inviteRequest.exists(resip::h_CallInfos))
		{
			// Iterate through Call-Info headers and look for answer-after=0 parameter
			for(resip::GenericUris::const_iterator i = inviteRequest.header(resip::h_CallInfos).begin();
				i != inviteRequest.header(resip::h_CallInfos).end(); i++)
			{
				if (i->exists(p_answerafter) && i->param(p_answerafter) == "0")
				{
					shouldAutoAnswer = true;
				}
			}
		}

		if (required) *required = autoAnswerRequired;

		return shouldAutoAnswer;
	}

}


namespace iSip
{

	ConversationProfile::ConversationProfile(const ConversationProfile &src) :
		Misc(src.Misc),
		m_sessionCaps(src.m_sessionCaps),
		m_natIpConverter(src.m_natIpConverter),
		m_autoAnswerProfile(src.m_autoAnswerProfile),
		m_secureMediaProfile(src.m_secureMediaProfile),
		m_secureChallengeProfile(src.m_secureChallengeProfile),
		m_natTraversalProfile(src.m_natTraversalProfile),
		m_forwardingProfile(src.m_forwardingProfile),
		m_userProfile(new resip::UserProfile(*src.m_userProfile)),
		m_registrationProfile(*m_userProfile, src.m_registrationProfile)
	{
		ESS_ASSERT (this != &src);
	}

	// -------------------------------------------------------------------------------------

	ConversationProfile::ConversationProfile() :
		m_userProfile(new resip::UserProfile()),
		m_registrationProfile(*m_userProfile)
	{
	}

	// -------------------------------------------------------------------------------------

	bool ConversationProfile::IsValid(std::string *desc) const
	{
		if (!Misc.IsValid()) return false;

		if (m_secureMediaProfile.GetSecureMediaMode() == SecureMediaProfile::SrtpDtls &&
			m_natTraversalProfile.RequireTurnAllocation())
		{
			if (desc != 0) *desc = "You cannot use SrtpDtls and a Turn allocation at the same time - disabling SrtpDtls!";
			return false;
		}

		return true;
	}

	// -------------------------------------------------------------------------------------

	resip::SharedPtr<resip::UserProfile> ConversationProfile::UserProfilePtr()
	{
		return m_userProfile;
	}

	// -------------------------------------------------------------------------------------

	resip::UserProfile& ConversationProfile::UserProfile()
	{
		return *m_userProfile;
	}

	// -------------------------------------------------------------------------------------

	const resip::UserProfile& ConversationProfile::UserProfile() const
	{
		return *m_userProfile;
	}

	// -------------------------------------------------------------------------------------

	resip::SdpContents& ConversationProfile::sessionCaps()
	{
	   return m_sessionCaps;
	}
	
	// -------------------------------------------------------------------------------------

	const resip::SdpContents& ConversationProfile::sessionCaps() const
	{
	   return m_sessionCaps;
	}
}