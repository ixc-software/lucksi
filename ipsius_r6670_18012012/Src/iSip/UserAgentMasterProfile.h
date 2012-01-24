#pragma once 

#include <rutil/TransportType.hxx>
#include <rutil/dns/DnsStub.hxx>
#include <resip/stack/SecurityTypes.hxx> 
#include <resip/dum/MasterProfile.hxx>
#include "SipUtils.h"

namespace iSip
{

	/**
	  This class extends the resip MasterProfile to include UserAgent 
	  specific settings.

	  Author: Scott Godin (sgodin AT SipSpectrum DOT com)
	*/

	class UserAgentMasterProfile : public resip::MasterProfile
	{
	public:  
		UserAgentMasterProfile();  

		class TransportInfo
		{
		public:

            bool IsCorrectProtocolType() const
            {
                return mProtocol == resip::TLS || mProtocol == resip::DTLS ||
                    mProtocol == resip::UDP || mProtocol == resip::TCP;
            }

			resip::TransportType mProtocol;
			int mPort;
			resip::IpVersion mIPVersion;
			resip::Data mIPInterface;
			resip::Data mSipDomainname;
			resip::SecurityTypes::SSLType mSslType;
		};

		/**
		 Adds a network transport to use for send/receiving SIP messages.

		 @note This MUST be called before the UserAgent is created

		 @param protocol Transport type: UDP,TCP or TLS
		 @param port     UDP or TCP port to listen on
		 @param version  IP Version: V4 or V6
		 @param ipInterface IP Interface to bind to - empty string
							binds to all interfaces
		 @param sipDomainname TLS Domain name - only used if protocol 
						  is TLS
		 @param sslType  SSLv23 vs TLSv1 - only use if protocol is TLS
		*/
		void addTransport( resip::TransportType protocol,
			int port, 
			resip::IpVersion version=resip::V4,
			const resip::Data& ipInterface = resip::Data::Empty, 
			const resip::Data& sipDomainname = resip::Data::Empty, // TLS only
			resip::SecurityTypes::SSLType sslType = resip::SecurityTypes::TLSv1 );

		/**
		 Gets a vector of the transports previously added.

		 @return Reference to a vector of TransportInfo's
		*/
		typedef std::vector<TransportInfo> TransportList;

		const TransportList& getTransports() const;

		/**
		 Adds a domain suffix used in ENUM DNS queries.  
		 For example: e164.arpa

		 @note This MUST be called before the UserAgent is created

		 @param enumSuffix ENUM Domain suffix to add
		*/
		void addEnumSuffix(const resip::Data& enumSuffix);

		/**
		 Gets a vector of the ENUM domain suffixes previously added.

		 @return Reference to a vector of ENUM domain suffixes
		*/
		const std::vector<resip::Data>& getEnumSuffixes() const;

		/**
		 Adds an additional DNS server to ones that are attempted
		 to be discovered by the SIP stack.

		 @note This MUST be called before the UserAgent is created

		 @param dnsServerIPAddress IP Address of DNS server to add
		*/
		void addAdditionalDnsServer(const resip::Data& dnsServerIPAddress);

		/**
		 Gets a vector of the DNS Servers previously added.

		 @note This does not include DNS servers detected by the stack

		 @return Reference to a vector of DNS server IP Addresses
		*/
		const resip::DnsStub::NameserverList& getAdditionalDnsServers() const;

		/**
		 Get/Set the location where the SIP stack will look for X509 
		 certificates

		 @note This MUST be called before the UserAgent is created

		 @return Data cert path location
		*/
		virtual resip::Data& certPath();
		virtual const resip::Data& certPath() const; 

		/**
		 Get/Set wether SIP message statistics are send to logging subsystem

		 @return bool Set to true to enable statistics
		*/
		virtual bool& statisticsManagerEnabled();
		virtual bool statisticsManagerEnabled() const; 


		/**
		 Get/Set the interval at which subscriptions are retried if
		 they fail (if one is not suggested in the failure).

		 @note This MUST be called before the UserAgent is created

		 @return int subscription retry interval in seconds
		*/
		virtual int& subscriptionRetryInterval();
		virtual int subscriptionRetryInterval() const;

		void ForceShutdownTimeoutMs(int val)	{	m_forceShutdownTimeoutMs = val; }
		int  ForceShutdownTimeoutMs() const		{	return m_forceShutdownTimeoutMs;}

		boost::function<SipUtils::PrintMode ()> ResipLogMode;

	private:            
		resip::Data mCertPath;
		bool mStatisticsManagerEnabled;
		TransportList mTransports;
		std::vector<resip::Data> mEnumSuffixes;
		resip::DnsStub::NameserverList mAdditionalDnsServers;
		int mSubscriptionRetryInterval;
		int m_forceShutdownTimeoutMs;
	};

}
