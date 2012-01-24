#pragma once

#include "stdafx.h"

#include "Domain/NamedObject.h"
#include "Domain/DomainClass.h"

#include "NObjSipTrustedNodes.h"
#include "SipTransportInfo.h"
#include "SipUtils.h"
#include "ISipAccessPoint.h"
#include "rutil/SharedPtr.hxx"

namespace iReg {	class CallRecord; }

namespace iSip
{
	class UserAgentMasterProfile;
	class IRemoteParticipantCreator;
	class IUserAgentRegistrator;
	class UserAgent;
	class NObjSipAccessPoint;

	class NObjSipUserAgent : public Domain::NamedObject 
	{
	public:
		Q_OBJECT;

		typedef NObjSipUserAgent T;
	public:

		NObjSipUserAgent(Domain::IDomain *pDomain, const Domain::ObjectName &name);

		~NObjSipUserAgent();
	
	// DRI
	public:

		Q_PROPERTY(bool Enabled READ m_isEnabled WRITE Enable);

		Q_INVOKABLE 
			void SetupTransport(iSip::SipTransportInfo::Type protocol, 
				int localPort = 0,
                const QString &ipInterface = "",
				iSip::SipTransportInfo::IpVersion ip = SipTransportInfo::Ip4);

		Q_INVOKABLE void AddAccessPoint(const QString &name, 
			const QString &registerUri,
			const QString &m_password = "");

		Q_INVOKABLE void DeleteAccessPoint(const QString &name);

		Q_PROPERTY(iSip::SipUtils::PrintMode ResipLogMode READ m_resipLogMode WRITE m_resipLogMode );

		Q_INVOKABLE void DumProcessingStat(DRI::ICmdOutput *pOutput);

		Q_PROPERTY(QString TransportInfo READ TransportInfo);

		Q_PROPERTY(int ForceShutdownTimeoutMs READ ForceShutdownTimeoutMs WRITE ForceShutdownTimeoutMs);

	public:

		IRemoteParticipantCreator* CallCreator();

		IUserAgentRegistrator* Registrator();

		SipUtils::PrintMode ResipLogMode() const {	return m_resipLogMode; }

		const UserAgentMasterProfile& GetUserAgentMasterProfile() const;

	private:

		void Enable(bool val);

		QString TransportInfo() const;

		void ForceShutdownTimeoutMs(int val);
		int  ForceShutdownTimeoutMs() const;

		void RegistratorLog(const QString &str);

	private:
		
		bool m_isEnabled;
		SipUtils::PrintMode m_resipLogMode;

		SipTrustedNodeList m_sipNodeList;

		NObjSipTrustedNodes m_trustedNodeDri;
		NObjSipTrustedNodes m_banNodeDri;

		resip::SharedPtr<UserAgentMasterProfile> m_profile;
		boost::shared_ptr<ConversationProfile>   m_defaultConversationProfile;

		boost::scoped_ptr<UserAgent> m_userAgentImpl;

		typedef std::vector<NObjSipAccessPoint*> AccessPoints;
		AccessPoints m_accessPoints;
	};
}


