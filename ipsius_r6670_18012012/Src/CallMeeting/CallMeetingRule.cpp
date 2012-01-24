#include "stdafx.h"
#include "CallMeetingRule.h"
#include "Utils/OutputFormatters.h"
#include "iLog/LogWrapper.h"

namespace CallMeeting
{
	CallMeetingRule::CallMeetingRule(const QString &country,
		const CallMeetingRulePar &par) :
		m_country(country),
		m_parameters(par),
		m_allowedAtBegin(0),
		m_allowedCall(0),
		m_currentIncomingCalls(0),
		m_currentAcceptedCalls(0)
	{
	}

	// -------------------------------------------------------------------------------

	void CallMeetingRule::Parameters(const CallMeetingRulePar &par)
	{
		m_parameters = par;
	}

	// -------------------------------------------------------------------------------

	CallMeetingRulePar CallMeetingRule::Parameters() const
	{
		return m_parameters;
	}

	// -------------------------------------------------------------------------------

	QString CallMeetingRule::Country() const
	{
		return m_country;
	}

	// -------------------------------------------------------------------------------

	QString CallMeetingRule::Brief() const
	{
		return QString("%1 All: %2; Accept: %3; Release: %4; Allow: %5")
			.arg(m_parameters.SaveRule("|"))
			.arg(m_currentIncomingCalls)
			.arg(m_currentAcceptedCalls)
			.arg(m_currentIncomingCalls - m_currentAcceptedCalls)
			.arg(m_allowedCall);
	}

	// -------------------------------------------------------------------------------

	void CallMeetingRule::RuleTimeoutExpired(std::string *logStr)
	{
		if(m_parameters.Percent() == 0) return;

		m_prevStat.clear();
		GetPeriodStat(m_prevStat);
		if(logStr != 0 && m_currentIncomingCalls != 0)
		{			
			*logStr += "Period info: " + m_prevStat.join("; ").toStdString();
		}
		if (m_currentIncomingCalls >= m_parameters.Density()) 
		{
			m_allowedCall += (m_currentIncomingCalls * m_parameters.Percent()) / 100;
		}
		m_currentIncomingCalls = 0;
		m_currentAcceptedCalls = 0;

		m_intervalTime.Reset();
	}

	// -------------------------------------------------------------------------------

	bool CallMeetingRule::CheckIncomingCall(boost::shared_ptr<CallMeetingStatList> stat)
	{
		stat->Push(m_stat);

		++m_currentIncomingCalls;

		if(m_allowedCall == 0) return false;

		--m_allowedCall;
		++m_currentAcceptedCalls;

		return true;
	}

	// -------------------------------------------------------------------------------

	void CallMeetingRule::Stat(const QString &customer, QStringList &out, int checkTimeoutSec) const
	{
		out << QString("Rule for customer '%1', country '%2'. Density %3, percent %4%").
			arg(customer).
			arg(m_country).
			arg(m_parameters.Density()).
			arg(m_parameters.Percent());
		if(!m_prevStat.isEmpty())	out << "Previous interval:" << m_prevStat;

		{

			int timeLeft = (Utils::TimerTicks::Ticks() - m_intervalTime.StartVal()) / 1000;
			out << " " << QString("Current interval. Time left: %1:%2").
				arg(timeLeft / 60).arg(timeLeft % 60);
			GetPeriodStat(out);
		}
		out << QString("General statistic.");
		m_stat.ToString(out);
	}

	// -------------------------------------------------------------------------------

	void CallMeetingRule::GetPeriodStat(QStringList &out) const
	{
		Utils::TableOutput table("  ", ' ');
		table.AddRow("All calls", QString("%1").arg(m_currentIncomingCalls));
		table.AddRow("Accepted calls", QString("%1").arg(m_currentAcceptedCalls));
		table.AddRow("Released calls", QString("%1").arg(m_currentIncomingCalls - m_currentAcceptedCalls));
		table.AddRow("Allowed calls", QString("%1").arg(m_allowedCall));
		out << table.Get();
	}

	// -------------------------------------------------------------------------------

	void CallMeetingRule::RestartStat()
	{
		m_stat.Restart();
	}

	// -------------------------------------------------------------------------------

	CallMeetingState::Rule CallMeetingRule::SaveState() const
	{
		CallMeetingState::Rule rule;
		rule.Country = m_country;
		rule.AllowedCall = m_allowedCall;
		rule.CurrentIncomingCalls = m_currentIncomingCalls;
		rule.CurrentAcceptedCalls = m_currentAcceptedCalls;

		return rule;
	}

	// -------------------------------------------------------------------------------

	void CallMeetingRule::UpdateSate(const CallMeetingState::Rule &rule)
	{
		ESS_ASSERT(rule.Country == m_country);

		m_allowedCall = rule.AllowedCall;
		m_currentIncomingCalls = rule.CurrentIncomingCalls;
		m_currentAcceptedCalls = rule.CurrentAcceptedCalls;
	}

};

