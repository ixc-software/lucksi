#include "stdafx.h"
#include "iSip/undeferrno.h"

#include "NObjSipAccessPoint.h"
#include "NObjSipUserAgent.h"
#include "ConversationProfile.h"
#include "UserAgentMasterProfile.h"
#include "ConversationManager.h"
#include "SipUtils.h"

#include "Utils/AtomicTypes.h"
#include "Utils/IntToString.h"
#include "Utils/GetDefLocalHost.h"
#include "rutil/SharedPtr.hxx"
#include "iReg/ISysReg.h"

namespace
{
	bool GUseInstanceId = true;

	QString HardwareAddress()
	{
		typedef QList<QNetworkInterface> Interfaces;

		Interfaces list(QNetworkInterface::allInterfaces ());
		
		for (Interfaces::const_iterator i = list.begin(); i != list.end(); ++i)
		{
			if (!i->hardwareAddress().isEmpty()) return i->hardwareAddress();
		}

		return "-";
	}

	std::string GenerateInstanceId()
	{
		static Utils::AtomicInt GNumber;

		return "i_" + HardwareAddress().remove(":").toStdString() + "_" + Utils::IntToString(GNumber.FetchAndAdd(1));
	}

	const iSip::UserAgentMasterProfile::TransportInfo&  GetAgentTransportInfo(
		const iSip::UserAgentMasterProfile &profile)
	{
		ESS_ASSERT (!profile.getTransports().empty());
		
		return profile.getTransports().front();
	}

	void UpdateRemoteParticipantPar(boost::shared_ptr<iSip::ConversationProfile> conversationProfile,
		iSip::ISipAccessPoint::Parameters &parameters)
	{
		ESS_ASSERT (parameters.IsValid());

		if (parameters.ConversationProfile == 0)
		{
			parameters.ConversationProfile.reset(new iSip::ConversationProfile(*conversationProfile));
		}

		if (!parameters.SourceUserName.empty())
		{

			if (parameters.UseSourceNameInContact)
			{
				parameters.ConversationProfile->UserProfile().setOverrideContactUserName(parameters.SourceUserName);
			}

			if (parameters.UseSourceUserNameInFrom)
			{
				parameters.ConversationProfile->UserProfile().getDefaultFrom().uri().user() = parameters.SourceUserName;

			}

			if (parameters.UseSourceUserNameAsDisplayName)
			{
				parameters.ConversationProfile->UserProfile().getDefaultFrom().displayName() = parameters.SourceUserName;
			}
		}

	}

}

namespace iSip
{
    NObjSipAccessPoint::NObjSipAccessPoint(NObjSipUserAgent &parent,
        const Domain::ObjectName &name,
        const UserAgentMasterProfile &userAgentProfile,
        const QString &registerUri,
        const QString &password)  :
        NamedObject(&parent.getDomain(), name, &parent),
        m_agent(parent),
        m_profile(CreateConversationProfile(userAgentProfile,
			0,
			boost::bind(&iReg::ISysReg::CallInfo, &getDomain().SysReg(), Name(), _1)))
    {
		
		RegistratorLog("Created.");

        m_isEnabled = true;

        m_isActivatedByOwner = false;

        m_isDefaultReceiver = false;

        m_enableOutCallWithoutRegistration = false;

        RegisterUri(registerUri);

        RegistrationTimeSec(180);

        RegistrationRetryTimeSec(180);

        Password(password);
    }


    // -------------------------------------------------------------------------------------

    NObjSipAccessPoint::~NObjSipAccessPoint()
    {
		RegistratorLog("Deleted.");
    }

    // -------------------------------------------------------------------------------------

    void NObjSipAccessPoint::Start()
    {
        m_isActivatedByOwner = true;

        if (m_isEnabled) EnableImpl(true);
    }


	// ---------------------------------------------------------------------
/*
	void NObjSipAccessPoint::SetupProtocol(SipTransportInfo::Type protocol, int port)
	{
		ESS_ASSERT (!IsActive());

		if (port == 0)
			m_profile->UserProfile().unsetFixedTransportPort();
		else
			m_profile->UserProfile().setFixedTransportPort(port);

		if (!m_profile->UserProfile().hasOverrideHostAndPort()) return;

		Uri uri;

		uri.param(resip::p_transport) = resip::toData(SipTransportInfo::ConvertToResip(protocol));

		uri.host() = m_profile->UserProfile().getOverrideHostAndPort().host();

		if (port != 0) uri.port() = port;

		m_profile->UserProfile().setOverrideHostAndPort(uri);
	}
*/
    // -------------------------------------------------------------------------------------

    void NObjSipAccessPoint::Stop()
    {
        m_isActivatedByOwner = false;

        if (m_isEnabled) EnableImpl(false);
    }

    // -------------------------------------------------------------------------------------

    void NObjSipAccessPoint::SetAsDefaultReceiver()
    {
        m_isDefaultReceiver = true;

		if (!IsActive()) return;

		if (!m_registrationItem.Empty()) Registrator().SetDefIncomingCallReceiver(m_registrationItem.Value());
    }

    // -------------------------------------------------------------------------------------

    boost::shared_ptr<ConversationProfile> NObjSipAccessPoint::
        CreateConversationProfile(const UserAgentMasterProfile &profile,
			int port,
            iReg::CallRecordWrapper::ObserverFn callRecordFn)
    {
        boost::shared_ptr<ConversationProfile> conversationProfile(new ConversationProfile());
	
		conversationProfile->UserProfile() = profile;

        conversationProfile->UserProfile().setDefaultRegistrationTime(0);

		conversationProfile->Misc.ResipLogMode(profile.ResipLogMode);
        conversationProfile->Misc.CallRecordFn(callRecordFn);

		if (GUseInstanceId)
		{
			conversationProfile->UserProfile().
				setInstanceId(SipUtils::ToResipData(GenerateInstanceId()));
		}

        // Create Session Capabilities and assign to coversation Profile
        resip::Data address(Utils::GetDefLocalIp().toStdString().c_str());
        // Build s=, o=, t=, and c= lines
        resip::SdpContents::Session::Origin origin("-", 0 /* sessionId */, 0 /* version */, resip::SdpContents::IP4, address);   // o=   Note:  sessionId and version will be replace in actual offer/answer
        resip::SdpContents::Session session(0, origin, "-" /* s= */);
        session.connection() = resip::SdpContents::Session::Connection(resip::SdpContents::IP4, address);  // c=
        session.addTime(resip::SdpContents::Session::Time(0, 0));

        // Build Codecs and media offering
        resip::SdpContents::Session::Medium medium("audio", port, 0, "RTP/AVP");
        //		SdpContents::Session::Codec g711ucodec("PCMU", 8000);
        //		g711ucodec.payloadType() = 0;  /* RFC3551 */ ;

        medium.addCodec(resip::SdpContents::Session::Codec::ALaw_8000);
        medium.addCodec(resip::SdpContents::Session::Codec::ULaw_8000);
        medium.addCodec(resip::SdpContents::Session::Codec::TelephoneEvent);

        medium.addAttribute("ptime", resip::Data(20));  // 20 ms of speech per frame (note G711 has 10ms samples, so this is 2 samples per frame)
        medium.addAttribute("sendrecv");
        session.addMedium(medium);

        resip::SdpContents sessionCaps;
        sessionCaps.session() = session;
        conversationProfile->sessionCaps() = sessionCaps;

        return conversationProfile;
    }

    // -------------------------------------------------------------------------------------

    bool NObjSipAccessPoint::IsUsingForOutgoingCalls() const
    {
        ESS_UNIMPLEMENTED;

        return false;
    }

    // -------------------------------------------------------------------------------------

    Utils::SafeRef<ISipAccessPoint> NObjSipAccessPoint::AccessPoint()
    {
        return this;
    }

	// -------------------------------------------------------------------------------------
	// ISipAccessPoint

	void NObjSipAccessPoint::SetCallReceiver(const std::string &callsReceiverName,
		Utils::WeakRef<ISipAccessPointEvents&> callsReceiver)
	{
		ESS_ASSERT (callsReceiverName.empty() == callsReceiver.Empty());

		m_callsReceiverName = callsReceiverName;
		m_callsReceiver = callsReceiver;

		if (m_callsReceiver.Empty())
			RegistratorLog("Clear call receiver.");
		else
			RegistratorLog(QString("Set call receiver '") + callsReceiverName.c_str() +  "'");
	}

	// -------------------------------------------------------------------------------------

	Utils::SafeRef<IRemoteParticipant> NObjSipAccessPoint::CreateOutgoingParticipant(
		Utils::SafeRef<IRemoteParticipantUser> user,
		boost::shared_ptr<Parameters> parameters)
	{
        if (EnableRegistration() && !m_enableOutCallWithoutRegistration)
        {
            if (m_registrationItem.Empty() ||
                m_registrationItem.Value().GetState() != SipRegistrationState::RsSuccess)
            {
                if (IsLogActive(InfoTag))
                {
                    Log(InfoTag) << "Registration incomplete. Release all outgoing calls" << iLogW::EndRecord;
                }

                return Utils::SafeRef<IRemoteParticipant>();
            }
        }

		IRemoteParticipantCreator *callCreator = m_agent.CallCreator();

		UpdateRemoteParticipantPar(m_profile, *parameters);


		return (callCreator == 0) ?
			Utils::SafeRef<IRemoteParticipant>() :
			callCreator->CreateOutgoingParticipant(user, parameters);
	}

	// ~ ISipAccessPoint

	// -------------------------------------------------------------------------------------
	// ISipAccessPointEventsExt

	void NObjSipAccessPoint::OnIncomingParticipant(const ISipAccessPoint *src,
		Utils::SafeRef<IRemoteParticipant> newParticipant,
		const resip::SipMessage &msg,
		bool autoAnswer,
		const ConversationProfile &conversationProfile)
	{
		if (m_callsReceiver.Empty())
		{
			newParticipant->DestroyParticipant(403,
				"Call receiver is missing. Destroy call. Message: " +
				SipUtils::ToString(conversationProfile.Misc.ResipLogMode(), msg));
			return;
		}

		m_callsReceiver.Value().OnIncomingParticipant(this, newParticipant, msg, autoAnswer, conversationProfile);

	}

    // -------------------------------------------------------------------------------------

	void NObjSipAccessPoint::OnRequestOutgoingParticipant(const ISipAccessPoint *src,
		Utils::SafeRef<IRemoteParticipant> newParticipant,
		const resip::SipMessage &msg,
		const ConversationProfile &conversationProfile)
	{
		newParticipant->DestroyParticipant(403,
			"OnRequestOutgoingParticipant isn't supported. Destroy call. Message: " +
			SipUtils::ToString(conversationProfile.Misc.ResipLogMode(), msg));
	}

    // -------------------------------------------------------------------------------------

	void NObjSipAccessPoint::RegistrationState(SipRegistrationState::Value state,
		const std::string &description)
	{
		RegistratorLog("Registration state: " + SipRegistrationState::ToString(state));
	}

    // -------------------------------------------------------------------------------------

    IUserAgentRegistrator& NObjSipAccessPoint::Registrator()
    {
        IUserAgentRegistrator *p = m_agent.Registrator();
        ESS_ASSERT (p != 0);
        return *p;
    }

    // -------------------------------------------------------------------------------------

    void NObjSipAccessPoint::Enable(bool val)
    {
        if (m_isEnabled == val) return;

        if (m_isActivatedByOwner) EnableImpl(val);

        m_isEnabled = val;
    }

    // -------------------------------------------------------------------------------------

    void NObjSipAccessPoint::EnableImpl(bool val)
    {
        if (!val)
        {
			RegistratorLog("Disabled.");

			if (!m_registrationItem.Empty()) m_registrationItem.Value().RemoveRegistration();
			
			m_registrationItem = Utils::WeakRef<IRegistrationItem&>();

            return;
        }

		ESS_ASSERT (m_registrationItem.Empty());
		
		RegistratorLog("Enabled. State: " + GetStateStr());

		{
			std::string desc;
			if (!m_profile->IsValid(&desc)) ThrowRuntimeException(desc.c_str());
		}

		m_registrationItem = Registrator().AddRegistration(
			Name().Name().toStdString(),
			*this,
			m_profile);

		ESS_ASSERT (!m_registrationItem.Empty());

        if (m_isDefaultReceiver) SetAsDefaultReceiver();
    }

	// -------------------------------------------------------------------------------------

	QString NObjSipAccessPoint::Realm() const
	{
		return "lkjaolifjqwpoijhufqpiuh";
	}

	// -------------------------------------------------------------------------------------

    SipRegistrationState::Value NObjSipAccessPoint::GetState() const
    {
		return (m_registrationItem.Empty()) ?
			SipRegistrationState::Disabled :
			m_registrationItem.Value().GetState();
    }

	// -------------------------------------------------------------------------------------

	QString NObjSipAccessPoint::GetStateStr() const
	{
		return SipRegistrationState::ToString(GetState());
	}

	// -------------------------------------------------------------------------------------

	void NObjSipAccessPoint::RegistratorLog(const QString &str)
	{
		if (IsLogActive(InfoTag))
		{
			Log(InfoTag) << str.toStdString() << iLogW::EndRecord;
		}

		getDomain().SysReg().EventInfo(Name(), str);
	}

    // -------------------------------------------------------------------------------------

    void NObjSipAccessPoint::RegisterUri(const QString  &val)
    {
        resip::Uri uri;
        std::string error;

        if (!SipUtils::ConvertStrToUri(val, uri, error)) ThrowRuntimeException(error);

        m_profile->UserProfile().setDefaultFrom(resip::NameAddr(uri));
    }

    // -------------------------------------------------------------------------------------

    QString NObjSipAccessPoint::RegisterUri() const
    {
        return SipUtils::ToQString(m_profile->UserProfile().getDefaultFrom().uri());
    }

	// ---------------------------------------------------------------------

	resip::Data ConvertIp(const resip::Data &externalIp, const resip::Data &ip)
	{
		return (externalIp.empty()) ? ip : externalIp;
	}

	void NObjSipAccessPoint::ExternalIp(const QString &par)
	{
		if (par.isEmpty())
		{
			m_profile->UserProfile().unsetOverrideHostAndPort();

			m_profile->ClearNatIpConverter();
			return;
		}
		
		Utils::HostInf::FromString(par.toStdString(), Utils::HostInf::HostOnly); // will throw if wrong par

		resip::Uri  uri;

		uri.host() = SipUtils::ToUtf8(par);

		UserAgentMasterProfile::TransportInfo transportInfo(
			GetAgentTransportInfo(m_agent.GetUserAgentMasterProfile()));

		uri.port() = transportInfo.mPort;

		if (transportInfo.mProtocol !=  resip::UDP)
			uri.param(resip::p_transport) = resip::toDataLower(transportInfo.mProtocol);

		m_profile->UserProfile().setOverrideHostAndPort(uri);

		m_profile->SetNatIpConverter(boost::bind(&ConvertIp, uri.host(), _1));
		
	}

	// ---------------------------------------------------------------------

	QString NObjSipAccessPoint::ExternalIp() const
	{
		if (!m_profile->UserProfile().hasOverrideHostAndPort()) return "";

		return SipUtils::ToQString(m_profile->UserProfile().getOverrideHostAndPort().host());
	}

    // -------------------------------------------------------------------------------------

    void NObjSipAccessPoint::Password(const QString  &val)
    {
        m_password = val;

        if (m_password.isEmpty())
            m_profile->UserProfile().clearDigestCredentials();
        else
        {
            m_profile->UserProfile().setDigestCredential(
                SipUtils::ToUtf8(Realm()),
                m_profile->UserProfile().getDefaultFrom().uri().user(),
                SipUtils::ToUtf8(m_password));
        }
    }

    // -------------------------------------------------------------------------------------

    QString NObjSipAccessPoint::Password() const
    {
        return m_password;
    }

    // -------------------------------------------------------------------------------------

    void NObjSipAccessPoint::EnableRegistration(bool val)
    {
        if (val && RegistrationTimeSec() == 0) ThrowRuntimeException("RegistrationTimeSec is null");

        m_profile->GetRegistrationProfile().EnableRegistration(val);
    }

    // -------------------------------------------------------------------------------------

    bool NObjSipAccessPoint::EnableRegistration() const
    {
        return m_profile->GetRegistrationProfile().EnableRegistration();
    }

    // -------------------------------------------------------------------------------------

    void NObjSipAccessPoint::RegistrationTimeSec(int val)
    {
        m_profile->GetRegistrationProfile().RegistrationTimeSec(val);
    }

    // -------------------------------------------------------------------------------------

    int NObjSipAccessPoint::RegistrationTimeSec() const
    {
        return m_profile->GetRegistrationProfile().RegistrationTimeSec();
    }

    // -------------------------------------------------------------------------------------

    void NObjSipAccessPoint::RegistrationRetryTimeSec(int val)
    {
        m_profile->GetRegistrationProfile().RegistrationRetryTimeSec(val);
    }

    // -------------------------------------------------------------------------------------

    int NObjSipAccessPoint::RegistrationRetryTimeSec() const
    {
        return m_profile->GetRegistrationProfile().RegistrationRetryTimeSec();
    }

    // -------------------------------------------------------------------------------------

    QString NObjSipAccessPoint::IncomingCallsDestination() const
    {
		return m_callsReceiverName.c_str();
    }

    // -------------------------------------------------------------------------------------

    void NObjSipAccessPoint::ClearCallsDestination()
    {
		SetCallReceiver("", Utils::WeakRef<ISipAccessPointEvents&>());
    }

    // -------------------------------------------------------------------------------------

    iSip::ForwardingType::Val NObjSipAccessPoint::GetForwardingType() const
    {
        return m_profile->GetForwardingProfile().Type;
    }

    // -------------------------------------------------------------------------------------

    void NObjSipAccessPoint::SetForwardingType(ForwardingType::Val val)
    {
        m_profile->GetForwardingProfile().Type = val;
    }

    // -------------------------------------------------------------------------------------

    QString NObjSipAccessPoint::ForwardingUri() const
    {
        return SipUtils::ToQString(m_profile->GetForwardingProfile().Destination.toString());
    }

    // -------------------------------------------------------------------------------------

    void NObjSipAccessPoint::ForwardingUri(const QString &val)
    {
        resip::Uri uri;
        std::string error;
        if (!SipUtils::ConvertStrToUri(val, uri, error)) ThrowRuntimeException(error);

        m_profile->GetForwardingProfile().Destination = uri;
    }

    // -------------------------------------------------------------------------------------

    int NObjSipAccessPoint::NoAnswerTimeoutSec() const
    {
        return m_profile->GetForwardingProfile().NoAnswerTimeout;
    }

    // -------------------------------------------------------------------------------------

    void NObjSipAccessPoint::NoAnswerTimeoutSec(int val)
    {
        m_profile->GetForwardingProfile().NoAnswerTimeout = val;
    }

	int NObjSipAccessPoint::FixedPort() const
	{
		return m_profile->UserProfile().getFixedTransportPort();
	}

}
