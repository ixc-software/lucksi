#include "stdafx.h"
#include "CallMeetingCustomer.h"
#include "Sip/ISipCall.h"

namespace CallMeeting
{
	CallMeetingCustomer::CallMeetingCustomer(Utils::SafeRef<iLogW::ILogSessionCreator> logCreator,
		const QString &name, 
		const QString &number) :
		m_name(name),
		m_number(number.toStdString())
	{
		m_log.reset(logCreator->CreateSession("Customer_" + name.toStdString(), false));
		m_ruleTag = m_log->RegisterRecordKind(L"Rule", true);

	}

	// -------------------------------------------------------------------------------

	void CallMeetingCustomer::RuleTimeoutExpired()
	{
		std::string logStr;
		std::string *logStrPtr = (m_log->LogActive(m_ruleTag)) ? 
			&logStr : 0;

		for(CallMeetingRules::iterator i = m_rules.begin(); 
			i != m_rules.end(); ++i)
		{
			i.value()->RuleTimeoutExpired(logStrPtr);
		}

		if(m_log->LogActive(m_ruleTag))
			*m_log << m_ruleTag << logStr << iLogW::EndRecord;

	}

	// -------------------------------------------------------------------------------

	QString CallMeetingCustomer::Name() const
	{
		return m_name;
	}

	// -------------------------------------------------------------------------------

	std::string CallMeetingCustomer::Number() const
	{
		return m_number;
	}

	// -------------------------------------------------------------------------------

	void CallMeetingCustomer::EnableTrace(bool enable)
	{
		m_log->LogActive(enable);
	}

	// -------------------------------------------------------------------------------

	void CallMeetingCustomer::SetRuleParameters(const QString &country, const CallMeetingRulePar &par)
	{	
		CallMeetingRule *rule = Rule(country);

		if(rule != 0)
		{
			rule->Parameters(par);
			return;
		}
		m_rules.insert(country, 
			boost::shared_ptr<CallMeetingRule>(
				new CallMeetingRule(country, par)));			
	}

	// -------------------------------------------------------------------------------

	void CallMeetingCustomer::ClearRules()
	{
		m_rules.clear();
	}

	// -------------------------------------------------------------------------------

	CallMeetingRule *CallMeetingCustomer::RuleForNewCall(const QString &country,
		boost::shared_ptr<CallMeetingStatList> stat)
	{
		stat->Push(m_stat);
		return Rule(country);
	}

	// -------------------------------------------------------------------------------

	void CallMeetingCustomer::StatAll(QStringList &out) const
	{
		out << QString("Customer: %1").arg(m_name);
		m_stat.ToString(out);
	}

	// -------------------------------------------------------------------------------

	void CallMeetingCustomer::StatShort(Utils::TableOutput &table) const
	{
		m_stat.ShortStat(m_name, table);
	}

	// -------------------------------------------------------------------------------

	void CallMeetingCustomer::StatByRule(QStringList &out, const QString &counry, int checkTimeoutSec) const
	{
		const CallMeetingRule *rule = Rule(counry);
		if(rule == 0)
		{
			out << QString("Rule for customer '%1', country '%2' not found").
				arg(m_name).arg(counry);
			return;
		}
		rule->Stat(m_name, out, checkTimeoutSec);
	}

	// -------------------------------------------------------------------------------

	void CallMeetingCustomer::RestartStat()
	{
		m_stat.Restart();
		for(CallMeetingRules::iterator i = m_rules.begin(); 
			i != m_rules.end(); ++i)
		{
			i.value()->RestartStat();
		}
	}

	// -------------------------------------------------------------------------------

	CallMeetingRulePar CallMeetingCustomer::RuleParameters(const QString &counry) const
	{
		const CallMeetingRule *rule = Rule(counry);
		return (rule == 0) ? CallMeetingRulePar() : rule->Parameters();
	}

	// -------------------------------------------------------------------------------

	CallMeetingState::Customer CallMeetingCustomer::SaveState() const
	{
		CallMeetingState::Customer customer;
		customer.Name = m_name;

		for (CallMeetingRules::const_iterator i = m_rules.begin();
			i != m_rules.end(); ++i)
		{
			CallMeetingState::Rule rule(i.value()->SaveState());

			if(!rule.IsEmpty()) customer.Rules.push_back(rule);
		}

		return customer;
	}

	// -------------------------------------------------------------------------------

	void CallMeetingCustomer::UpdateSate(const CallMeetingState::Customer &customer)
	{
		ESS_ASSERT(customer.Name == m_name);

		for (CallMeetingState::RuleList::const_iterator i = customer.Rules.begin();
			i != customer.Rules.end(); ++i)
		{
			CallMeetingRule *rule = Rule(i->Country);

			if (rule != 0) rule->UpdateSate(*i);
		}
	}

	// -------------------------------------------------------------------------------

	const CallMeetingRule* CallMeetingCustomer::Rule(const QString &country) const
	{
		CallMeetingRules::const_iterator i = m_rules.find(country);

		if (i == m_rules.end()) return 0;

		ESS_ASSERT(i.value()->Country() == country);

		return i.value().get();
	}

	// -------------------------------------------------------------------------------

	CallMeetingRule *CallMeetingCustomer::Rule(const QString &country)
	{
		CallMeetingRules::iterator i = m_rules.find(country);

		if (i == m_rules.end()) return 0;

		ESS_ASSERT(i.value()->Country() == country);

		return i.value().get();
	}

};

