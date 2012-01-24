#include "stdafx.h" 
#include "CallMeetingStat.h"
#include "Utils/OutputFormatters.h"

namespace {
	class StatHelper
	{
	public:
		StatHelper(const Utils::StatElementForInt &stat) : m_stat(stat){}
		
		QString Min() const
		{
			return (m_stat.Empty()) ? "0" : QString("%1").arg(m_stat.Min()); 
		}
		QString Max() const
		{
			return (m_stat.Empty()) ? "0" : QString("%1").arg(m_stat.Max()); 
		}
		QString Average() const
		{
			return (m_stat.Empty()) ? "0" : QString("%1").arg(m_stat.Average()); 
		}

	private:
		const Utils::StatElementForInt &m_stat;
	};
};

namespace CallMeeting
{
	CallMeetingStat::CallMeetingStat()
	{
		m_currentCalls = 0;
		m_currentTalks = 0;
		Restart();
	}

	// -------------------------------------------------------------------------------

	void CallMeetingStat::Restart()
	{
		m_activation = QDateTime::currentDateTime();
		m_lastCall = QDateTime();
		m_lastTalk = QDateTime();

		m_totalCalls = m_currentCalls;
		m_totalTalks = m_currentTalks;

		m_destinationBusy = 0;
		m_canceledCalls = 0;
		m_relByTimeout = 0;
		m_maxTalkDuration = 0;

		m_waitingDuration.Reset();
		m_talkDuration.Reset();
		m_cancelDuration.Reset();
	}

	// -------------------------------------------------------------------------------

	int CallMeetingStat::TotalCalls() const
	{
		return m_totalCalls;
	}

	// -------------------------------------------------------------------------------

	void CallMeetingStat::IncCurrentCalls()
	{
		m_totalCalls++;
		m_currentCalls++;
		m_lastCall = QDateTime::currentDateTime();
	}

	// -------------------------------------------------------------------------------

	void CallMeetingStat::DecCurrentCalls()
	{
		ESS_ASSERT(m_currentCalls != 0);
		m_currentCalls--;
	}

	// -------------------------------------------------------------------------------

	int  CallMeetingStat::CurrentCalls() const
	{
		return m_currentCalls;
	}

	// -------------------------------------------------------------------------------

	void CallMeetingStat::IncCurrentTalks()
	{
		m_totalTalks++;
		m_currentTalks++;
		m_lastTalk = QDateTime::currentDateTime();
	}

	// -------------------------------------------------------------------------------

	void CallMeetingStat::DecCurrentTalks()
	{
		ESS_ASSERT(m_currentTalks != 0);
		m_currentTalks--;
	}

	// -------------------------------------------------------------------------------

	void CallMeetingStat::IncDestinationBusy()
	{
		m_destinationBusy++;
	}

	// -------------------------------------------------------------------------------

	void CallMeetingStat::IncRelByTimeout()
	{
		m_relByTimeout++;
	}

	// -------------------------------------------------------------------------------

	void CallMeetingStat::IncMaxTalkDuration()
	{
		m_maxTalkDuration++;
	}

	// -------------------------------------------------------------------------------

	void CallMeetingStat::IncCanceledCalls()
	{
		m_canceledCalls++;
	}

	// -------------------------------------------------------------------------------

	void CallMeetingStat::AddWaitAnswerDuration(int timeout)
	{
		m_waitingDuration.Add(timeout);
	}

	// -------------------------------------------------------------------------------

	void CallMeetingStat::AddCancelDuration(int timeout)
	{
		m_cancelDuration.Add(timeout);
	}

	// -------------------------------------------------------------------------------

	void CallMeetingStat::AddTalkDuration(int timeout)
	{
		m_talkDuration.Add(timeout);
	}

	// -------------------------------------------------------------------------------

	void CallMeetingStat::ToTable(Utils::TableOutput &table) const
	{
		table.AddRow("Total calls", QString("%1").arg(m_totalCalls));
		table.AddRow("Total talks", QString("%1").arg(m_totalTalks));
		table.AddRow("Current calls", QString("%1").arg(m_currentCalls));
		table.AddRow("Current talks", QString("%1").arg(m_currentTalks));

		table.AddRow("Min talk duration (sec)", StatHelper(m_talkDuration).Min());
		table.AddRow("Max talk duration (sec)", StatHelper(m_talkDuration).Max());
		table.AddRow("Average talk duration (sec)", StatHelper(m_talkDuration).Average());

		table.AddRow("Min wait answer duration (sec)", StatHelper(m_waitingDuration).Min());
		table.AddRow("Max wait answer  duration (sec)", StatHelper(m_waitingDuration).Max());
		table.AddRow("Average wait answer  duration (sec)", StatHelper(m_waitingDuration).Average());

		table.AddRow("Canceled calls", QString("%1").arg(m_canceledCalls));
		table.AddRow("Min canceled call duration (sec)", StatHelper(m_cancelDuration).Min());
		table.AddRow("Max canceled call duration (sec)", StatHelper(m_cancelDuration).Max());
		table.AddRow("Average canceled call duration (sec)", StatHelper(m_cancelDuration).Average());

		table.AddRow("Destination busy", QString("%1").arg(m_destinationBusy));
		table.AddRow("Rejected without talk by timeout", QString("%1").arg(m_relByTimeout));
		table.AddRow("Max talk duration", QString("%1").arg(m_maxTalkDuration));
	}

	// -------------------------------------------------------------------------------

	void CallMeetingStat::ToString(QStringList &out) const
	{
		CallHeader(out);
		Utils::TableOutput table("  ", ' ');
		ToTable(table);
		out << table.Get();
	}

	// -------------------------------------------------------------------------------

	void CallMeetingStat::HeadShortStat(Utils::TableOutput &table)
	{
		table.AddRow("Filter",  "Total calls", "Total talks", "Curr calls", "Curr talks");
	}

	// -------------------------------------------------------------------------------

	void CallMeetingStat::TimerHeader(QStringList &out) const
	{
		out << QString("Started: %1").arg(m_activation.toString(Qt::SystemLocaleShortDate));
		out << QString("Current: %1").arg(QDateTime::currentDateTime().toString(Qt::SystemLocaleShortDate));
	}

	// -------------------------------------------------------------------------------

	void CallMeetingStat::CallHeader(QStringList &out) const
	{
		TimerHeader(out);

		if(!m_lastCall.isNull())
		{
			out << QString("Last call started at: %1").arg(m_lastCall.toString(Qt::SystemLocaleShortDate));
		}
		if(!m_lastTalk.isNull())
		{
			out << QString("Last talk started at: %1").arg(m_lastTalk.toString(Qt::SystemLocaleShortDate));
		}
	}

	// -------------------------------------------------------------------------------

	void CallMeetingStat::ShortStat(const QString &name, Utils::TableOutput &table) const
	{
		if(m_totalCalls == 0) return;
			
		table.AddRow(name,  
			QString("%1").arg(m_totalCalls), 
			QString("%1").arg(m_totalTalks), 
			QString("%1").arg(m_currentCalls), 
			QString("%1").arg(m_currentTalks));
	}

};

