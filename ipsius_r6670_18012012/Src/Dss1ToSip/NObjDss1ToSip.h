#pragma once

#include "stdafx.h"
#include "Domain/NamedObject.h"
#include "Domain/DomainClass.h"
#include "Utils/ManagedList.h"
#include "Utils/WeakRef.h"
#include "Calls/StatisticCalls.h"
#include "iSip/ISipAccessPoint.h"
#include "Dss1Def.h"
#include "GateBoardInfoProc.h"
#include "IGateDss1.h"

namespace iReg	{	class CallRecord; }
namespace iSip	{	class NObjSipOutCallProfile; }

namespace Dss1ToSip
{
	class NObjDss1;
	class Dss1ToSipCall;
	class NObjDss1InCallProfile;

	class NObjDss1ToSip : public Domain::NamedObject,
		public iSip::ISipAccessPointEvents,
		IGateDss1Events
	{
		Q_OBJECT;
		typedef NObjDss1ToSip T;

	public:

		NObjDss1ToSip(Domain::IDomain *pDomain, const Domain::ObjectName &name); 
		~NObjDss1ToSip();

		Q_PROPERTY(bool Enabled READ m_isEnabled WRITE Enable);
		Q_PROPERTY(bool ActiveTrunkCount READ m_activeTrunkCount);

		Q_INVOKABLE void AddDss1(const QString &hardType, 
			const QString &name, 
			const QString &boardAlias, 
			const QString &sideType, 
			int intfNumber = 0, int DChannelSlot = 16);

		Q_INVOKABLE void DeleteDss1(const QString &name);

		Q_INVOKABLE void Calls(DRI::ICmdOutput *pOutput, bool brief = false);
		Q_INVOKABLE void Info(DRI::ICmdOutput *pOutput);

		Q_INVOKABLE void Stat(DRI::ICmdOutput *pOutput);
		Q_INVOKABLE void RestartStat();
		
		Q_INVOKABLE void WaitWhileNoactive(DRI::IAsyncCmd *pAsyncCmd, int maxTimeout);

		Q_PROPERTY(QString SipPoint READ m_sipPointName WRITE SipPoint);

//		Q_PROPERTY(NumberingPlan::Value Dss1NumberingPlan READ m_numberingPlan WRITE m_numberingPlan);
//		Q_PROPERTY(TypeOfNumber::Value Dss1TypeOfNumber READ m_typeOfNumber WRITE m_typeOfNumber);

	// iSip::ISipAccessPointEvents
	private:

		void OnIncomingParticipant(const iSip::ISipAccessPoint *src,
			Utils::SafeRef<iSip::IRemoteParticipant> callSip,
			const resip::SipMessage &msg,
			bool autoAnswer, 
			const iSip::ConversationProfile &conversationProfile);

		void OnRequestOutgoingParticipant(const iSip::ISipAccessPoint *src,
			Utils::SafeRef<iSip::IRemoteParticipant> callSip, 
			const resip::SipMessage &msg, 
			const iSip::ConversationProfile &conversationProfile);

	// IGateDss1Events
	private: 

		void Activated(const IGateDss1 *); 
		void Deactivated(const IGateDss1 *);
		void IncomingCall(const IGateDss1 *,
			Utils::SafeRef<IDss1Call> call,
			const ISDN::DssPhoneNumber &calledAddress,
			const ISDN::DssPhoneNumber &callingAddress);

	// property 
	private:

		void Enable(bool par) {	(par) ? Enable() : Disable(); }
		void Enable();
		void Disable();
		void SipPoint(const QString &val);
		void CheckCorrectProperty();

	private:

		void CallRegistrator(Dss1ToSipCall *call, bool addToList); 
		void OnDeleteCall(Dss1ToSipCall *call){}

		void OnWaitActive(iCore::MsgTimer *pT);
		NObjDss1 *AllocDss1Link();

	private:        

		iSip::NObjSipOutCallProfile *m_outCallProfile;
		NObjDss1InCallProfile		*m_inCallProfile;	

		Utils::WeakRefHost m_selfHost;

		bool m_isEnabled;
		Calls::StatisticCalls m_stat;

		NumberingPlan::Value m_numberingPlan;
		TypeOfNumber::Value m_typeOfNumber;

		iCore::MsgTimer m_waitActiveTimer;
		
		QString m_sipPointName;
		Utils::SafeRef<iSip::ISipAccessPoint> m_sipPoint;

		int m_activeTrunkCount;
		typedef std::vector<NObjDss1*> ListDss1Link;
		ListDss1Link m_dss1;
		Utils::SafeRef<NObjDss1> m_lastDss1Trunk;

		typedef boost::ptr_list<Dss1ToSipCall> ListOfCalls;
		ListOfCalls m_calls;
	};
}


