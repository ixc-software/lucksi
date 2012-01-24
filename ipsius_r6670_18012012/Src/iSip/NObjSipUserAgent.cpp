#include "stdafx.h"
#include "undeferrno.h"
#include "NObjSipUserAgent.h"
#include "NObjSipAccessPoint.h"
#include "UserAgent.h"
#include "UserAgentMasterProfile.h"
#include "SipUtils.h"
#include "iSip/NObjResipLog.h"
#include "Utils/IntToString.h"
#include "Utils/GetDefLocalHost.h"
#include "BuildInfo.h"
#include "iReg/ISysReg.h"


namespace 
{
	using namespace iSip;

	resip::SharedPtr<UserAgentMasterProfile> createUserAgentMasterProfile()
	{
		resip::SharedPtr<UserAgentMasterProfile> profile(new UserAgentMasterProfile);

		std::string userAgentName = "ipsius rev-" + Utils::IntToString(AutoBuildInfo::Revision());
		profile->setUserAgent(userAgentName.c_str());

		// Settings
		profile->statisticsManagerEnabled() = false;
		profile->validateContentEnabled() = false;
		profile->validateContentLanguageEnabled() = false;
		profile->validateAcceptEnabled() = false;
		profile->clearSupportedLanguages();
		profile->addSupportedLanguage(resip::Token("en"));  
		profile->clearSupportedMimeTypes();
		profile->addSupportedMimeType(resip::INVITE, resip::Mime("application", "sdp"));
		profile->addSupportedMimeType(resip::OPTIONS,resip::Mime("application", "sdp"));
		profile->addSupportedMimeType(resip::UPDATE, resip::Mime("application", "sdp"));  
		profile->addSupportedMimeType(resip::NOTIFY, resip::Mime("message", "sipfrag"));  
		profile->clearSupportedMethods();
		profile->addSupportedMethod(resip::INVITE);
		profile->addSupportedMethod(resip::ACK);
		profile->addSupportedMethod(resip::CANCEL);
		profile->addSupportedMethod(resip::OPTIONS);
		profile->addSupportedMethod(resip::BYE);
		profile->addSupportedMethod(resip::REFER);    
		profile->addSupportedMethod(resip::NOTIFY);    
		profile->addSupportedMethod(resip::SUBSCRIBE); 
		profile->addSupportedMethod(resip::UPDATE);    
		profile->clearSupportedOptionTags();
		profile->addSupportedOptionTag(resip::Token(resip::Symbols::Replaces));      
		profile->addSupportedOptionTag(resip::Token(resip::Symbols::Timer)); 
		profile->addSupportedOptionTag(resip::Token(resip::Symbols::NoReferSub));
		profile->addSupportedOptionTag(resip::Token(resip::Symbols::AnswerMode));
		profile->addSupportedOptionTag(resip::Token(resip::Symbols::TargetDialog));
		//profile->addSupportedOptionTag(Token(Symbols::C100rel));  
		profile->clearSupportedSchemes();
		profile->addSupportedScheme("sip");  
#ifdef USE_SSL
		profile->addSupportedScheme("sips");
#endif

		// Have stack add Allow/Supported/Accept headers to INVITE dialog establishment messages
		profile->clearAdvertisedCapabilities(); // Remove Profile Defaults, then add our preferences
		profile->addAdvertisedCapability(resip::Headers::Allow);  
		//profile->addAdvertisedCapability(Headers::AcceptEncoding);  // This can be misleading - it might specify what is expected in response
		profile->addAdvertisedCapability(resip::Headers::AcceptLanguage);  
		profile->addAdvertisedCapability(resip::Headers::Supported);  
		profile->setMethodsParamEnabled(true);

		return profile;
	}

}

namespace iSip
{
	
	NObjSipUserAgent::NObjSipUserAgent(Domain::IDomain *pDomain, const Domain::ObjectName &name) : 
		Domain::NamedObject(pDomain, name),
		m_isEnabled(false),
		m_resipLogMode(SipUtils::Short),
		m_trustedNodeDri(this, "TrustedNodeList", m_sipNodeList.TrustedList()),
		m_banNodeDri(this, "BanNodeList", m_sipNodeList.BanList()),
		m_profile(createUserAgentMasterProfile())
	{
		m_profile->ResipLogMode = boost::bind(&T::ResipLogMode, this);

		m_defaultConversationProfile = NObjSipAccessPoint::CreateConversationProfile(
			*m_profile, 
			0, 
			boost::bind(&iReg::ISysReg::CallInfo, &getDomain().SysReg(), Name(), _1));

		RegistratorLog("Created.");
	}

	// -------------------------------------------------------------------------------------

	NObjSipUserAgent::~NObjSipUserAgent()
	{
		RegistratorLog("Deleted.");
	}

	// -------------------------------------------------------------------------------------

	void NObjSipUserAgent::SetupTransport(SipTransportInfo::Type protocol, 
		int localPort,
        const QString &ipInterface,
		SipTransportInfo::IpVersion ip)
	{
		if (!m_profile->getTransports().empty()) ThrowRuntimeException("Transport is already defined");
			
		if (IsLogActive(InfoTag)) 
		{
			Log(InfoTag) << "Setup transport. "
				<< SipTransportInfo::ToString(ip) << " "
				<< SipTransportInfo::ToString(protocol) << " "
				<< localPort << " " 
                << ipInterface
				<< iLogW::EndRecord;
		}

		m_profile->addTransport(SipTransportInfo::ConvertToResip(protocol),
			localPort, 
			SipTransportInfo::ConvertToResip(ip),
            SipUtils::ToUtf8(ipInterface));		
	}

	// -------------------------------------------------------------------------------------

	void NObjSipUserAgent::AddAccessPoint(const QString &name, 
		const QString &registerUri,
		const QString &password)
	{
		NObjSipAccessPoint *item = new NObjSipAccessPoint(*this, 
			name,
			*m_profile,
			registerUri, 
			password);

		m_accessPoints.push_back(item);	

		if (m_isEnabled) item->Start();
	}

	// -------------------------------------------------------------------------------------

	void NObjSipUserAgent::DeleteAccessPoint(const QString &name)
	{
		AccessPoints::iterator i = m_accessPoints.begin();
		while (i != m_accessPoints.end())
		{
			ESS_ASSERT(*i != 0);
			if((*i)->Name().Name() == name) break;
			++i;
		}

		if(i == m_accessPoints.end()) ThrowRuntimeException("Cannot remove '" + name + "': No such object!");

		if((*i)->IsUsingForOutgoingCalls()) ThrowRuntimeException("Object is connected to user for outgoing calls");

		(*i)->Stop();

		delete *i;

		m_accessPoints.erase(i);
	}

	// -------------------------------------------------------------------------------------

	void NObjSipUserAgent::DumProcessingStat(DRI::ICmdOutput *pOutput)
	{
		if (m_userAgentImpl == 0) ThrowRuntimeException("Object disabled");
		
		pOutput->Add(m_userAgentImpl->DumProceessingStat().c_str());
	}

	// -------------------------------------------------------------------------------------

	IRemoteParticipantCreator* NObjSipUserAgent::CallCreator()
	{
		return  (!m_isEnabled) ? 0 : &m_userAgentImpl->CallCreator();
	}

	// -------------------------------------------------------------------------------------

	IUserAgentRegistrator* NObjSipUserAgent::Registrator()
	{
		return  (!m_isEnabled) ? 0 : &m_userAgentImpl->Registrator();
	}

	// -------------------------------------------------------------------------------------

	const UserAgentMasterProfile& NObjSipUserAgent::GetUserAgentMasterProfile() const
	{
		return *m_profile;
	}

	// -------------------------------------------------------------------------------------

	void NObjSipUserAgent::Enable(bool val)
	{
		if (m_isEnabled == val) return;

		if (!val) 
		{
			for(AccessPoints::iterator i = m_accessPoints.begin();
				i != m_accessPoints.end(); ++i)
			{
				(*i)->Stop();
			}

			m_userAgentImpl.reset();
		
			m_isEnabled = false;

			RegistratorLog("Disabled.");
			
			return;
		}

		if (m_profile->getTransports().empty())
		{
			m_profile->addTransport(resip::UDP, 0, resip::V4);		
		}

		if (m_accessPoints.empty()) ThrowRuntimeException("Registrations list is empty");

		m_isEnabled = true;

		NObjResipLog::CreateResipLog(*this);

		m_userAgentImpl.reset(new UserAgent(getMsgThread(), 
			Log(), 
			m_sipNodeList,
			m_profile, 
			m_defaultConversationProfile));

		std::string error;
		if (!m_userAgentImpl->Startup(error)) ThrowRuntimeException(error);

		RegistratorLog("Enabled. " + TransportInfo());

		for(AccessPoints::iterator i = m_accessPoints.begin();
			i != m_accessPoints.end(); ++i)
		{
			(*i)->Start();
		}
	}

	// -------------------------------------------------------------------------------------

	QString NObjSipUserAgent::TransportInfo() const
	{
		const UserAgentMasterProfile::TransportList &list = (m_userAgentImpl != 0) ?
			m_userAgentImpl->getTransports() :
			m_profile->getTransports();

		if (list.empty()) return "";

		const UserAgentMasterProfile::TransportInfo &info = list.front();

		std::ostringstream out;
		
		QAbstractSocket::NetworkLayerProtocol qtIpVersion = (info.mIPVersion == resip::V4) ? 
			QAbstractSocket::IPv4Protocol : QAbstractSocket::IPv6Protocol;

		out << resip::getTransportNameFromType(info.mProtocol)
			<< ((info.mIPVersion == resip::V4) ? " Ipv4 " : "Ipv6 ")
			<< SipUtils::ToString(info.mIPInterface)
			<< ":"
			<< info.mPort;

		return out.str().c_str();
	}

	// -------------------------------------------------------------------------------------

	void NObjSipUserAgent::ForceShutdownTimeoutMs(int val)	
	{	
		m_profile->ForceShutdownTimeoutMs(val); 
	}

	// -------------------------------------------------------------------------------------

	int  NObjSipUserAgent::ForceShutdownTimeoutMs() const		
	{	
		return m_profile->ForceShutdownTimeoutMs();
	}

	// -------------------------------------------------------------------------------------

	void NObjSipUserAgent::RegistratorLog(const QString &str)
	{
		if (IsLogActive(InfoTag))
		{
			Log(InfoTag) << str.toStdString() << iLogW::EndRecord;
		}

		getDomain().SysReg().EventInfo(Name(), str);
	}

}
