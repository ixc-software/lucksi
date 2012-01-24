#include "stdafx.h"
#include "Utils/ExeName.h"
#include "Utils/QSyncDelete.h"
#include "Utils/ErrorsSubsystem.h"
#include "NetThreadMonitor.h"

namespace iNet
{
	QString NetStatistic::ToString() const
	{
		return QString("Write event: %1; Read event: %2").arg(m_write).arg(m_read);
	}

	// ---------------------------------------------

	NetThreadMonitor::NetThreadMonitor() : 
		m_isActive(false),
		m_timer(this),
		m_file(OutputFileName())
	{
		connect(&m_timer, SIGNAL(timeout()), this, SLOT(TimerDone()));
		connect(this, SIGNAL(StartTimerSignal(int)), this, SLOT(StartTimerSlot(int)));
		connect(this, SIGNAL(StopTimerSignal()), this, SLOT(StopTimerSlot()));
	}
	
	// ---------------------------------------------

	void NetThreadMonitor::Start(int timeout) // throw
	{
		ESS_ASSERT(!m_isActive);
		ESS_ASSERT(timeout != 0);
		m_isActive = true;
		if(!m_file.open(QIODevice::WriteOnly)) 
		{
			ESS_THROW_MSG(FileError, "Cant open the file: " + OutputFileName().toStdString());		
		}
		StartTimerSignal(timeout);
	}

	// ---------------------------------------------

	void NetThreadMonitor::Stop()
	{
		ESS_ASSERT(m_isActive);
		m_isActive = false;
		m_file.close();
		StopTimerSignal();
	}

	// ---------------------------------------------

	QString NetThreadMonitor::OutputFileName() 
	{
		return QString(Utils::ExeName::GetExeDir().c_str()) + "NetMonitor.txt";
	}

	// ---------------------------------------------

	void NetThreadMonitor::StartTimerSlot(int timeout)
	{
		m_timer.start(timeout);
	}

	// ---------------------------------------------

	void NetThreadMonitor::StopTimerSlot()
	{
		m_timer.stop();
	}

	// ----------------------------------------------------------------------------------------

	void NetThreadMonitor::TimerDone()
	{
		if(!m_isActive) return;
		{
			QTextStream out(&m_file); 
			out << QDateTime::currentDateTime().toString(Qt::LocalDate) << "\n"
				<< "Tcp stat: " << TcpStat().ToString() << "\n"
				<< "Udp stat: " << UdpStat().ToString() << "\n\n";
		}
		TcpStat().Clear();
		UdpStat().Clear();
		m_file.flush();
	}

	// ---------------------------------------------

	NetStatistic &NetThreadMonitor::TcpStat()
	{
		static NetStatistic tcpStat;
		return tcpStat;
	}

	// ---------------------------------------------

	NetStatistic &NetThreadMonitor::UdpStat()
	{
		static NetStatistic udpStat;
		return udpStat;
	}
};


