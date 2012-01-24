#include "stdafx.h" 
#include "StatisticCalls.h"

namespace Calls
{
 
	StatisticCalls::Direction::Direction() : 
		m_currentCalls(0), 
		m_allCalls(0),
		m_currentTalks(0),
		m_allTalks(0)
	{}

	// -------------------------------------------------------------------------------

	void StatisticCalls::Direction::Restart()
	{
		m_allCalls = m_currentCalls;
		m_allTalks = m_currentTalks;
	}

	// -------------------------------------------------------------------------------

	void StatisticCalls::Direction::Add()
	{
		m_currentCalls++;
		m_allCalls++;
	}

	// -------------------------------------------------------------------------------

	void StatisticCalls::Direction::Dec()
	{
		m_currentCalls--;
	}

	// -------------------------------------------------------------------------------

	void StatisticCalls::Direction::AddTalk()
	{
		m_currentTalks++;
		m_allTalks++;
	}

	// -------------------------------------------------------------------------------

	void StatisticCalls::Direction::DecTalk()
	{
		m_currentTalks--;
	}

	// -------------------------------------------------------------------------------

	void StatisticCalls::Direction::ToTable(const QString &dir, 
		Utils::TableOutput &table) const
	{
		table.AddRow(dir, 
			QString("%1").arg(m_allCalls), 
			QString("%1").arg(m_allTalks), 
			QString("%1").arg(m_currentCalls), 
			QString("%1").arg(m_currentTalks));
	}

	// -------------------------------------------------------------------------------

	void StatisticCalls::Direction::Add(const Direction &val)
	{
		m_allCalls += val.m_allCalls;
		m_currentCalls += val.m_currentCalls;
		m_allTalks += val.m_allTalks;
		m_currentTalks += val.m_currentTalks;
	}

	// -------------------------------------------------------------------------------

	StatisticCalls::Call::Call(StatisticCalls::Direction::SafeRef statistic) : 
		m_talk(false), m_statistic(statistic)
	{
		m_statistic->Add();
	}

	// -------------------------------------------------------------------------------

	StatisticCalls::Call::~Call()
	{
		m_statistic->Dec();
		if(m_talk)
			m_statistic->DecTalk();
	}

	// -------------------------------------------------------------------------------

	void StatisticCalls::Call::Talk()
	{
		m_talk = true;
		m_statistic->AddTalk();
	}

	// -------------------------------------------------------------------------------

	StatisticCalls::StatisticCalls()
	{
		Restart();
	}

	// -------------------------------------------------------------------------------

	void StatisticCalls::Restart()
	{
		m_activation = QDateTime::currentDateTime();
		m_incoming.Restart();
		m_outgoing.Restart();
	}

	// -------------------------------------------------------------------------------

	QStringList StatisticCalls::ToTable() const
	{
		QStringList res;
		res << QString("Started: %1").arg(m_activation.toString(Qt::SystemLocaleShortDate));
		res << QString("Current: %1").arg(QDateTime::currentDateTime().toString(Qt::SystemLocaleShortDate));
		Utils::TableOutput table("  ", ' ');
		table.AddRow("Dir", "All calls", "All talks", "Curr calls", "Curr talks");
		m_outgoing.ToTable("Out", table);
		m_incoming.ToTable("In ", table);

		Direction all;
		all.Add(m_incoming);
		all.Add(m_outgoing);
		all.ToTable("All", table);
		res << table.Get();
		return res;
	}


};

