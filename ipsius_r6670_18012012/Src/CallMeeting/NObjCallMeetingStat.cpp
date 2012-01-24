#include "stdafx.h"

#include "NObjCallMeeting.h"
#include "Utils/OutputFormatters.h"


namespace CallMeeting
{
	NObjCallMeeting::Statistic::Statistic()
	{
		m_unknownCountry = 0;
		m_unknownIp = 0;
		m_unknownUserA = 0;
	}

	// -------------------------------------------------------------------------------

	void NObjCallMeeting::Statistic::Restart()
	{
		m_unknownCountry = 0;
		m_unknownIp = 0;
		m_unknownUserA = 0;
		m_base.Restart();
	}

	// -------------------------------------------------------------------------------

	void NObjCallMeeting::Statistic::ToString(QStringList &out) const
	{
		m_base.CallHeader(out);
		Utils::TableOutput table("  ", ' ');
		m_base.ToTable(table);

		table.AddRow("Unknown country", QString("%1").arg(m_unknownCountry));
		table.AddRow("Unknown ip of source", QString("%1").arg(m_unknownIp));
		table.AddRow("Unknown user A", QString("%1").arg(m_unknownUserA));
		out << table.Get();
	}

	// -------------------------------------------------------------------------------

	void NObjCallMeeting::Statistic::IncUnknownCountry()
	{
		++m_unknownCountry;
	}

	// -------------------------------------------------------------------------------

	void NObjCallMeeting::Statistic::IncUnknownIp()
	{
		++m_unknownIp;
	}

	// -------------------------------------------------------------------------------

	void NObjCallMeeting::Statistic::IncUnknownUserA()
	{
		++m_unknownUserA;
	}

	// -------------------------------------------------------------------------------

	CallMeetingStat &NObjCallMeeting::Statistic::Base()
	{
		return m_base;
	}
};


