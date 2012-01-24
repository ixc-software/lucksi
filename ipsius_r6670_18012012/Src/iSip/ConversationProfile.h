#pragma once 

// #include <resip/dum/UserProfile.hxx>
#include <resip/stack/SdpContents.hxx>
#include <rutil/SharedPtr.hxx>

#include "Utils/ErrorsSubsystem.h"
#include "ForwardingProfile.h"
#include "SipUtils.h"
#include "iReg/CallRecord.h"

namespace resip 
{	
	class Profile; 
	class UserProfile; 
}

namespace iSip
{

	class RegistrationProfile
	{
	public:

		RegistrationProfile(resip::UserProfile &userProfile);

		RegistrationProfile(resip::UserProfile &userProfile, 
			const RegistrationProfile &src);

		void EnableRegistration(bool val);

		bool EnableRegistration() const;

		void RegistrationTimeSec(int val);

		int RegistrationTimeSec() const;

		void RegistrationRetryTimeSec(int val);

		int RegistrationRetryTimeSec() const;
	
	private:
		
		resip::UserProfile &m_userProfile;

		bool m_enableRegistration;
		int m_registrationTime;
		int m_registrationRetryTime;
	};


	class AutoAnswerProfile 
	{
	public:

		AutoAnswerProfile()
		{
			m_allowAutoAnswer = false;
			m_allowPriorityAutoAnswer = false;
		}

		/**
		 Set whether auto answers are allowed, if an autoanswer indication
		 is found in a new inbound call request.
		*/  
		void SetAllowAutoAnswer(bool val) { m_allowAutoAnswer = val; }

		/**
		 Set whether priority auto answers are allowed, if a priority 
		 autoanswer indication is found in a new inbound call request.
		*/  
		void SetAllowPriorityAutoAnswer(bool val) { m_allowPriorityAutoAnswer = val; }

		/** 
		 Checks an Invite message for auto-answer headers against the current allow 
		 settings.  
		 
		 @param inviteRequest SIP Invite message to check
		 @param required Will return true if the auto-answer is "required"
		*/
		bool ShouldAutoAnswer(const resip::SipMessage &inviteRequest, bool *required = 0) const;

	private:
		bool m_allowAutoAnswer;
		bool m_allowPriorityAutoAnswer;
	};

	// -------------------------------------------------------------------------------------

	class SecureChallengeProfile
	{
	public:  

		SecureChallengeProfile()
		{
			m_challengeAutoAnswerRequests = false;
			m_challengeOODReferRequests	  = false;
		}

		/** 
		 Get/Set wether auto answer requests should be digest challenged
		 or not.

		 @note Requests will be challenged using the digest credentials
			   set on this profile
		 
		 @return bool True if auto answer requests should be challenged
		*/
		void ChallengeAutoAnswerRequests(bool val)  { m_challengeAutoAnswerRequests = val; }
		bool ChallengeAutoAnswerRequests() const	{ return m_challengeAutoAnswerRequests; }

		/** 
		 Get/Set wether out of dialog REFER requests should be digest 
		 challenged or not.

		 @note Requests will be challenged using the digest credentials
			   set on this profile
		 
		 @return bool True if OOD REFER requests should be challenged
		*/
		void ChallengeOODReferRequests(bool val){ m_challengeOODReferRequests = val; }
		bool ChallengeOODReferRequests() const	{ return m_challengeOODReferRequests; }

	private:            

		bool m_challengeAutoAnswerRequests;
		bool m_challengeOODReferRequests;
	};

	// -------------------------------------------------------------------------------------

	class SecureMediaProfile
	{
	public:  

		enum SecureMediaMode
		{
			NoSecureMedia, // Will accept secure media offers, but will not offer secure media in calls placed
			Srtp,       // SRTP with keying outside of media stream - ie. SDES key negotiation via SDP
			SrtpDtls    // SRTP with DTLS key negotiation
		};

		enum SecureMediaCryptoSuite
		{
			SRTP_AES_CM_128_HMAC_SHA1_32,
			SRTP_AES_CM_128_HMAC_SHA1_80
		};

		SecureMediaProfile()
		{
			m_secureMediaRequired = false;
			m_secureMediaMode = NoSecureMedia;
			m_defaultSecureMediaCryptoSuite = SRTP_AES_CM_128_HMAC_SHA1_80;
		}

		/** 
		 Get/Set the secure media mode that will be used for sending/receiving media packets.
		 NoSecureMedia - don't use any secure media strategies - RTP packets are sent 
						 unencrypted via the specified transport.
		 Srtp          - use SRTP with keying outside of media stream - ie. SDES key negotiation via SDP (default)
		 SrtpDtls      - use SRTP with DTLS key negotiation

		 @note If TurnTlsAllocation NatTraversalMode is used, then media will be secured from 
			   this UA to the TURN the turn server, even if NoSecureMedia is used.

		 @return SecureMediaMode
		*/
		void SetSecureMediaMode(SecureMediaMode &val) { m_secureMediaMode = val; }
		SecureMediaMode GetSecureMediaMode() const { return m_secureMediaMode; }

		/** 
		 Get/Set the wether Secure Media is required (default is false).
		 - if required then SAVP transport protocol is signalled in SDP offers
		 - if not required then AVP transport portocol is signalled in SDP offers 
		   and encryption=optional attribute is added

		 @return true if secure media is required
		*/
		void SetSecureMediaRequired(bool val) { m_secureMediaRequired = val; }
		bool SecureMediaRequired() const { return m_secureMediaRequired; }


		/** 
		 Get/Set the secure media default crypto suite.  The default crypto suite is used when
		 forming SDP offers (SDES only - does not apply to DTLS-SRTP).
		 SRTP_AES_CM_128_HMAC_SHA1_32 - Counter Mode AES 128 bit encryption with 
										32bit authenication code 
		 SRTP_AES_CM_128_HMAC_SHA1_80 - Counter Mode AES 128 bit encryption with 
										80bit authenication code (default)

		 @return SecureMediaMode
		*/
		bool SetSecureMediaDefaultCryptoSuite(SecureMediaCryptoSuite &val) 
		{ 
			return m_defaultSecureMediaCryptoSuite = val; 
		}

		SecureMediaCryptoSuite GetSecureMediaDefaultCryptoSuite() const 
		{ 
			return m_defaultSecureMediaCryptoSuite; 
		}

	private:            

		SecureMediaMode m_secureMediaMode;
		bool m_secureMediaRequired;
		SecureMediaCryptoSuite m_defaultSecureMediaCryptoSuite;
	};

	// -------------------------------------------------------------------------------------

	class NatTraversalProfile  
	{
	public:  

		/** 
		 The NAT traversal mode 
		 NoNatTraversal - don't use any NAT traversal strategies - local address and port
						  are used in SDP negotiations
		 StunBindDiscovery - use Binding discovery on a STUN server, to discover and use
							 "public" address and port in SDP negotiations
		 TurnUdpAllocation - Use a TURN server as a media relay.  Communicate to the TURN
							 server over UDP and Allocate a UDP relay address and port to 
							 use in SDP negotiations
		 TurnTcpAllocation - Use a TURN server as a media relay.  Communicate to the TURN
							 server over TCP and Allocate a UDP relay address and port to 
							 use in SDP negotiations
		 TurnTlsAllocation - Use a TURN server as a media relay.  Communicate to the TURN
							 server over TLS and Allocate a UDP relay address and port to 
							 use in SDP negotiations
		*/

		enum NatTraversalMode
		{
			NoNatTraversal,
			StunBindDiscovery,
			TurnUdpAllocation,
			TurnTcpAllocation,
			USE_SSL,
			TurnTlsAllocation
		};

		NatTraversalProfile() :
			m_mode(NoNatTraversal),
			m_serverPort(0)
		{
		}

		void SetMode(NatTraversalMode mode) { m_mode = mode; }
		NatTraversalMode GetMode() const { return m_mode; }

		bool RequireTurnAllocation() const 
		{
			return m_mode == TurnUdpAllocation ||
				m_mode == TurnTcpAllocation ||
				m_mode == TurnTlsAllocation; 
		}

		/** 
		 Get/Set the NAT traversal server hostname that will be used for if natTranversalMode
		 is anything other than NoNatTraversal.  The server can be specified as an IP address
		 or a A-Records DNS resolvable hostname.  SRV resolution is not yet supported.
		 
		 @return The server to use for NAT traversal
		*/
		void ServerHostname(const resip::Data &val) 
		{ 
			m_serverHostname = val; 
		}
		
		const resip::Data& ServerHostname() const 
		{ 
			return m_serverHostname; 
		}

		/** 
		 Get/Set the NAT traversal server port that will be used for if natTranversalMode
		 is anything other than NoNatTraversal.  
		 
		 @return The server port to use for NAT traversal
		*/
		void ServerPort(unsigned int port) 
		{ 
			m_serverPort = port; 
		}

		unsigned int ServerPort() const 
		{ 
			return m_serverPort; 
		}

		/** 
		 Get/Set the STUN username that will be used for if natTranversalMode
		 is anything other than NoNatTraversal.  The stun username/password is used for 
		 both STUN and TURN server authentication.
		 
		 @return The STUN username
		*/
		void StunUsername(const resip::Data &stunUsername) 
		{ 
			m_stunUsername = stunUsername; 
		}

		const resip::Data& StunUsername() const 
		{ 
			return m_stunUsername; 
		}

		/** 
		 Get/Set the STUN password that will be used for if natTranversalMode
		 is anything other than NoNatTraversal.  The stun username/password is used for 
		 both STUN and TURN server authentication.
		 
		 @return The STUN password
		*/
		void StunPassword(resip::Data &password) 
		{ 
			m_stunPassword = password; 
		}

		const resip::Data& StunPassword() const 
		{ 
			return m_stunPassword; 
		}

	private:            

		NatTraversalMode m_mode;
		resip::Data m_serverHostname;
		unsigned short m_serverPort;
		resip::Data m_stunUsername;
		resip::Data m_stunPassword;
	};

	// -------------------------------------------------------------------------------------

	class MiscProfile
	{
	public:

		bool IsValid() const
		{
            return !m_callRecordFn.empty() && !m_resipLogMode.empty();
		}

        void CallRecordFn(iReg::CallRecordWrapper::ObserverFn fn)
		{
			ESS_ASSERT (!fn.empty());
			
            m_callRecordFn = fn;
		}

        iReg::CallRecordWrapper::ObserverFn CallRecordFn()
		{
            ESS_ASSERT (!m_callRecordFn.empty());
            return m_callRecordFn;
		}

		void ResipLogMode(boost::function<SipUtils::PrintMode ()> fn)
		{
			ESS_ASSERT (!fn.empty());

			m_resipLogMode = fn;
		}

		SipUtils::PrintMode ResipLogMode() const 
		{
			ESS_ASSERT (!m_resipLogMode.empty());
			return m_resipLogMode();
		}

	private:
        iReg::CallRecordWrapper::ObserverFn m_callRecordFn;
		boost::function<SipUtils::PrintMode ()> m_resipLogMode;


	};

	// -------------------------------------------------------------------------------------

	class ConversationProfile 
	{
	public:  

		ConversationProfile();

		ConversationProfile(const ConversationProfile &src);

		bool IsValid(std::string *desc = 0) const;

		resip::UserProfile& UserProfile();
		const resip::UserProfile& UserProfile() const;

		resip::SharedPtr<resip::UserProfile> UserProfilePtr();
		
		/**
		 Get/Set the session capabilities.  Session capabilites are expressed
		 as a SessionDesriptionProtocol body.  Typically this is populated as you
		 would expect an SDP offer to look like that is generated by this application.

		 This is used to generate SDP offers and form valid SDP answers.

		 @return SdpContents representing the session capabilities
		*/  
		virtual resip::SdpContents& sessionCaps();
		virtual const resip::SdpContents& sessionCaps() const; 

		RegistrationProfile& GetRegistrationProfile() {	return m_registrationProfile; }
		const RegistrationProfile& GetRegistrationProfile()const {	return m_registrationProfile; }

		const AutoAnswerProfile& GetAutoAnswerProfile()	  const {	return m_autoAnswerProfile; } 

		const SecureMediaProfile& GetSecureMediaProfile() const {	return m_secureMediaProfile; } 

		const SecureChallengeProfile& GetSecureChallengeProfile() const 
		{
			return m_secureChallengeProfile;
		}

		const NatTraversalProfile& GetNatTraversalProfile() const
		{
			return m_natTraversalProfile;
		}

		const ForwardingProfile& GetForwardingProfile() const
		{
			return m_forwardingProfile;
		}

		ForwardingProfile& GetForwardingProfile()
		{
			return m_forwardingProfile;
		}

		typedef boost::function<resip::Data (const resip::Data&)> NatIpConverter;

		void SetNatIpConverter(NatIpConverter converter) 
		{
			ESS_ASSERT (!converter.empty());

			m_natIpConverter = converter;
		}

		void ClearNatIpConverter() 
		{
			m_natIpConverter.clear();
		}

		NatIpConverter GetNatIpConverter() const 
		{
			return m_natIpConverter;
		}

		MiscProfile Misc;
		
	private:            

		resip::SdpContents m_sessionCaps;  
		NatIpConverter		m_natIpConverter;
		AutoAnswerProfile	m_autoAnswerProfile;
		SecureMediaProfile	m_secureMediaProfile;
		SecureChallengeProfile m_secureChallengeProfile;
		NatTraversalProfile m_natTraversalProfile;
		ForwardingProfile   m_forwardingProfile;
		resip::SharedPtr<resip::UserProfile> m_userProfile;
		RegistrationProfile m_registrationProfile;
	};

}

