#pragma once 

#include "stdafx.h"
#include "Utils/SafeRef.h"
#include "Utils/TimerTicks.h"
#include "CallMeetingState.h"
#include "CallMeetingStat.h"
#include "CallMeetingRulePar.h"

namespace Sip
{
	class ISipCall;
	class SipMessageHandler;
};

namespace iLogW	{	class LogSession;	}
namespace Utils	{	class TableOutput;	}

namespace CallMeeting
{

	class CallMeetingRule : boost::noncopyable
	{
	public:

		CallMeetingRule(const QString &country, 
			const CallMeetingRulePar &par = CallMeetingRulePar());

		void Parameters(const CallMeetingRulePar &par);

		CallMeetingRulePar Parameters() const;

		QString Country() const;
		QString Brief() const;

		void RuleTimeoutExpired(std::string *logStr = 0);

		bool CheckIncomingCall(boost::shared_ptr<CallMeetingStatList> stat);
		

		void Stat(const QString &customer, QStringList &out, int checkTimeoutSec) const;
		void GetPeriodStat(QStringList &out) const;
		void RestartStat();

		CallMeetingState::Rule SaveState() const;

		void UpdateSate(const CallMeetingState::Rule &rule);

	private:
		Utils::TimerTicks m_intervalTime;
		CallMeetingStat m_stat;
		QStringList m_prevStat;
		QString m_country;
		
		CallMeetingRulePar m_parameters;

		int m_allowedAtBegin;
		int m_allowedCall;
		int m_currentIncomingCalls;
		int m_currentAcceptedCalls;
	};
};



