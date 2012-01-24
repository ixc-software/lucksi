#pragma once 

#include "stdafx.h"
#include "Utils/SafeRef.h"
#include "Utils/ManagedList.h"
#include "iLog/LogWrapper.h"
#include "CallMeetingStat.h"
#include "CallMeetingRule.h"

namespace Sip
{
	class ISipCall;
	class SipMessageHandler;
};

namespace Utils	{	class TableOutput;	}

namespace CallMeeting
{

	class CallMeetingCustomer : boost::noncopyable
	{
	public:
		
		CallMeetingCustomer(Utils::SafeRef<iLogW::ILogSessionCreator> logCreator,
			const QString &name, const QString &number);

		void RuleTimeoutExpired();
		
		QString Name() const;
		std::string Number() const;

		void EnableTrace(bool enable);

		void SetRuleParameters(const QString &country, const CallMeetingRulePar &par);

		void ClearRules();

		CallMeetingRule* RuleForNewCall(const QString &country,
			boost::shared_ptr<CallMeetingStatList> stat);
		
		void StatAll(QStringList &out) const;
		void StatShort(Utils::TableOutput &table) const;	
		void StatByRule(QStringList &out, const QString &counry, int checkTimeoutSec) const;
		void RestartStat();
		
		CallMeetingRulePar RuleParameters(const QString &counry) const;

		CallMeetingState::Customer 
			SaveState() const;

		void UpdateSate(const CallMeetingState::Customer &customer);

	private:	
		const CallMeetingRule *Rule(const QString &counry) const;
		CallMeetingRule *Rule(const QString &counry);

	private:
		boost::scoped_ptr<iLogW::LogSession> m_log;
		iLogW::LogRecordTag m_ruleTag;
		QString m_name;
		std::string m_number;
		CallMeetingStat m_stat;
		typedef QMap<QString, boost::shared_ptr<CallMeetingRule> > CallMeetingRules;
		CallMeetingRules m_rules;
	};
};


