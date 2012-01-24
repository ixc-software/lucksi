#pragma once

#include "stdafx.h"

#include "AutoTrainerSipCall.h"

#include "iCore/MsgTimer.h"
#include "Domain/NamedObject.h"
#include "Domain/DomainClass.h"
#include "DRI/INonCreatable.h"
#include "ISipAccessPoint.h"
#include "Utils/WeakRef.h"
#include "Utils/ManagedList.h"

namespace iSip
{

	class NObjSipAutotrainer : public Domain::NamedObject,
		public ISipAccessPointEvents
	{

		Q_OBJECT;

		typedef NObjSipAutotrainer T;

	public:

		NObjSipAutotrainer(Domain::IDomain *pDomain, const Domain::ObjectName &name);

		~NObjSipAutotrainer();

		Q_PROPERTY(QString SipPoint READ m_sipAccessPointName WRITE SipPoint);

		enum Commands
		{
			Dial,
			Wait,
			Alerting,
			Answer,
			RedirectTo,
			Hold,
			Unhold,
			Close
		};

		Q_ENUMS(Commands);

		Q_PROPERTY(QString CommandsForOutCall READ m_commandsOutStr);

		Q_INVOKABLE void StartOutgoingCall();

		Q_INVOKABLE void AddCommandForOutCall(Commands type, const QString &par = "");

		Q_INVOKABLE void ClearCommandsForOutCall();

		Q_PROPERTY(QString CommandsForInCall READ m_commandsInStr);

		Q_INVOKABLE void AddCommandForInCall(Commands type, const QString &par = "");
		
		Q_INVOKABLE void ClearCommandsForInCall();

	// ISipAccessPointEvents
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
	
	private:

		void SipPoint(const QString &val);

	private:

		void AddCommandToList(QString &commandsStr, AutotrainerSipCall::Commands &commands,
			Commands type, const QString &par);

		void ConvertStrToUri(const QString &val, resip::Uri &uri); 

		void OnOutCallsTimeout(int count);

		void DeleteCall(AutotrainerSipCall *call);

		void OnDeleteCall(AutotrainerSipCall *call) {}

	private:

		Utils::WeakRefHost m_selfHost;

		QString m_sipAccessPointName;

		Utils::SafeRef<ISipAccessPoint> m_sipAccessPoint;
		
		QString m_commandsOutStr;

		AutotrainerSipCall::Commands m_commandsOutCall;

		QString m_commandsInStr;

		AutotrainerSipCall::Commands m_commandsInCall;

		typedef Utils::ManagedList<AutotrainerSipCall> Calls;

		Calls m_calls;
	};
}

