#pragma once

#include "stdafx.h"

#include "ISipAccessPoint.h"
#include "IUserAgentRegistrator.h"
#include "ForwardingProfile.h"
#include "SipTransportInfo.h"

#include "Domain/NamedObject.h"
#include "Domain/DomainClass.h"
#include "DRI/INonCreatable.h"

#include "Utils/SafeRef.h"
#include "Utils/WeakRef.h"
#include "rutil/SharedPtr.hxx"

namespace iReg	{	class CallRecord; }

namespace iSip
{
	class ConversationProfile;
	class UserAgentMasterProfile;
	class NObjSipUserAgent;

	class NObjSipAccessPoint : public Domain::NamedObject,
		public DRI::INonCreatable,
		ISipAccessPointEventsExt,
		public ISipAccessPoint
	{
	public:
		Q_OBJECT;

		typedef NObjSipAccessPoint T;

	public:

		NObjSipAccessPoint(NObjSipUserAgent &parent, 
			const Domain::ObjectName &name,
			const UserAgentMasterProfile &profile,
			const QString &registerUri,
			const QString &m_password = "");

		~NObjSipAccessPoint();

		void Start();

		void Stop();

	// DRI
	public:

		Q_PROPERTY(bool Enabled READ m_isEnabled WRITE Enable);

		Q_PROPERTY(iSip::SipRegistrationState::Value State READ GetState);

		Q_PROPERTY(QString RegisterUri READ RegisterUri WRITE RegisterUri);

		Q_PROPERTY(QString ExternalIp READ ExternalIp WRITE ExternalIp);

		Q_PROPERTY(QString Password READ Password WRITE Password);

		Q_PROPERTY(bool DefaultForIncomingCall READ m_isDefaultReceiver);

		Q_INVOKABLE void SetAsDefaultReceiver();

		// Q_PROPERTY(int FixedPort READ FixedPort);


		Q_PROPERTY(bool EnableRegistration  
			READ EnableRegistration WRITE EnableRegistration);

		Q_PROPERTY(int RegistrationTimeSec 
			READ RegistrationTimeSec WRITE RegistrationTimeSec);

		Q_PROPERTY(int RegistrationRetryTimeSec 
			READ RegistrationRetryTimeSec WRITE RegistrationRetryTimeSec);

        Q_PROPERTY(bool EnableOutCallWithoutRegistration  
            READ m_enableOutCallWithoutRegistration WRITE m_enableOutCallWithoutRegistration);

		Q_PROPERTY(QString IncomingCallsDestination READ IncomingCallsDestination);

		Q_INVOKABLE void ClearCallsDestination();

		Q_PROPERTY(iSip::ForwardingType::Val ForwardingType 
			READ GetForwardingType WRITE SetForwardingType);

		Q_PROPERTY(QString ForwardingUri
			READ ForwardingUri WRITE ForwardingUri);

		Q_PROPERTY(int NoAnswerTimeoutSec READ NoAnswerTimeoutSec WRITE NoAnswerTimeoutSec);

	public:

		static boost::shared_ptr<ConversationProfile> 
			CreateConversationProfile(const UserAgentMasterProfile &profile, 
				int port, 
                iReg::CallRecordWrapper::ObserverFn callRecordFn);
	
		bool IsUsingForOutgoingCalls() const;

		Utils::SafeRef<ISipAccessPoint> AccessPoint();

	// ISipAccessPoint
	private:

		void SetCallReceiver(const std::string &callsReceiverName,
			Utils::WeakRef<ISipAccessPointEvents&> callsReceiver);

		Utils::SafeRef<IRemoteParticipant> CreateOutgoingParticipant(
			Utils::SafeRef<IRemoteParticipantUser> user,
			boost::shared_ptr<Parameters> parameters);

	// ISipAccessPointEventsExt
	private:

		void OnIncomingParticipant(const ISipAccessPoint *src,
			Utils::SafeRef<IRemoteParticipant> newParticipant,
			const resip::SipMessage &msg, 
			bool autoAnswer, 
			const ConversationProfile &conversationProfile);

		void OnRequestOutgoingParticipant(const ISipAccessPoint *src,
			Utils::SafeRef<IRemoteParticipant> newParticipant, 
			const resip::SipMessage &msg, 
			const ConversationProfile &conversationProfile);

		void RegistrationState(SipRegistrationState::Value state, 
			const std::string &description);

	private:

		IUserAgentRegistrator& Registrator();

		bool IsActive() const {	return m_isActivatedByOwner && m_isEnabled; }
		void Enable(bool val);

		void EnableImpl(bool val);

		QString Realm() const;

		SipRegistrationState::Value GetState() const;
		
		QString GetStateStr() const;

		void RegistratorLog(const QString &str);

	private:

		void RegisterUri(const QString  &val);
		QString RegisterUri() const;

		void ExternalIp(const QString &ip);
		QString ExternalIp() const;

		void Password(const QString  &val);
		QString Password() const;

		void EnableRegistration(bool val);
		bool EnableRegistration() const;

		void RegistrationTimeSec(int val);
		int RegistrationTimeSec() const;

		void RegistrationRetryTimeSec(int val);
		int RegistrationRetryTimeSec() const;

		QString IncomingCallsDestination() const;

		iSip::ForwardingType::Val GetForwardingType() const;
		void SetForwardingType(iSip::ForwardingType::Val val);
		
		QString ForwardingUri() const;
		void ForwardingUri(const QString &val);

		int NoAnswerTimeoutSec() const;
		void NoAnswerTimeoutSec(int val);

		int FixedPort() const;

	private:

		NObjSipUserAgent &m_agent;

		boost::shared_ptr<ConversationProfile> m_profile;

		bool m_isActivatedByOwner;

		bool m_isEnabled;

		bool m_isDefaultReceiver;

        bool m_enableOutCallWithoutRegistration;

		QString m_password;

        Utils::WeakRef<IRegistrationItem&> m_registrationItem;

		std::string m_callsReceiverName;
		Utils::WeakRef<ISipAccessPointEvents&> m_callsReceiver;
	};
}

