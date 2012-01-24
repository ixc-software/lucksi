#include "stdafx.h"
#include "iSip/undeferrno.h"

#include "UserAgentMasterProfile.h"
#include <resip/stack/Tuple.hxx>

namespace iSip
{
	using namespace resip;

	UserAgentMasterProfile::UserAgentMasterProfile() : 
		mStatisticsManagerEnabled(false),
		mSubscriptionRetryInterval(60)
	{
		#ifdef WIN32
			mCertPath = ".";
		#else
			mCertPath = getenv("HOME");
			mCertPath += "/.sipCerts/";
		#endif

		m_forceShutdownTimeoutMs = 200;
	}

	// -------------------------------------------------------------------------------------

	void UserAgentMasterProfile::addTransport( TransportType protocol,
		int port, 
		IpVersion version,
		const Data& ipInterface, 
		const Data& sipDomainname,
		SecurityTypes::SSLType sslType)
	{
		TransportInfo info;

		info.mProtocol = protocol;
		info.mPort = port;
		info.mIPVersion = version;
		info.mIPInterface = ipInterface;
		info.mSipDomainname = sipDomainname;
		info.mSslType = sslType;

		mTransports.push_back(info);
	}

	// -------------------------------------------------------------------------------------

	const UserAgentMasterProfile::TransportList& 
		UserAgentMasterProfile::getTransports() const
	{
		return mTransports;
	}
	
	// -------------------------------------------------------------------------------------

	void UserAgentMasterProfile::addEnumSuffix( const Data& enumSuffix)
	{
		mEnumSuffixes.push_back(enumSuffix);
	}

	// -------------------------------------------------------------------------------------

	const std::vector<Data>& UserAgentMasterProfile::getEnumSuffixes() const
	{
		return mEnumSuffixes;
	}

	// -------------------------------------------------------------------------------------

	void UserAgentMasterProfile::addAdditionalDnsServer( const Data& dnsServerIPAddress)
	{
		mAdditionalDnsServers.push_back(Tuple(dnsServerIPAddress, 0, UNKNOWN_TRANSPORT).toGenericIPAddress());
	}

	// -------------------------------------------------------------------------------------

	const DnsStub::NameserverList& 
		UserAgentMasterProfile::getAdditionalDnsServers() const
	{
		return mAdditionalDnsServers;
	}

	// -------------------------------------------------------------------------------------

	Data& UserAgentMasterProfile::certPath()
	{
		return mCertPath;
	}

	// -------------------------------------------------------------------------------------

	const Data& UserAgentMasterProfile::certPath() const
	{
		return mCertPath;
	}

	// -------------------------------------------------------------------------------------

	bool& UserAgentMasterProfile::statisticsManagerEnabled()
	{
		return mStatisticsManagerEnabled;
	}

	// -------------------------------------------------------------------------------------

	bool UserAgentMasterProfile::statisticsManagerEnabled() const
	{
		return mStatisticsManagerEnabled;
	}

	// -------------------------------------------------------------------------------------

	int& UserAgentMasterProfile::subscriptionRetryInterval()
	{
		return mSubscriptionRetryInterval;
	}

	// -------------------------------------------------------------------------------------

	int UserAgentMasterProfile::subscriptionRetryInterval() const
	{
		return mSubscriptionRetryInterval;
	}
}
