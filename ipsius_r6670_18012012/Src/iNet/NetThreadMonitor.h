#ifndef NET_THREAD_MONITOR_H_
#define NET_THREAD_MONITOR_H_

#include "qtimer.h"
#include "Utils/ErrorsSubsystem.h"

namespace Utils { class ThreadSyncEvent; };

namespace iNet
{
	class NetStatistic
	{
	public:
		NetStatistic()
		{
			Clear();
		}
		void Clear()
		{
			m_write = 0;
			m_read = 0;
		}
		QString ToString() const;
		void IncWrite() {	++m_write; }
		void IncRead()	{	++m_read; }
	private:
		int m_write;
		int m_read;
	};


	// можно хранить только по указателю, 
	// после вызова Delete указатель на объект не валиден, выполняется процедура удаления
	class NetThreadMonitor :public QObject,
		 boost::noncopyable
	{
		Q_OBJECT
	public:
        ESS_TYPEDEF(FileError);
		NetThreadMonitor();
		
		void Start(int timeout); // throw
		void Stop();
		static QString OutputFileName();

		static NetStatistic &TcpStat();
		static NetStatistic &UdpStat();
	signals:
		void StartTimerSignal(int timeout);
		void StopTimerSignal();
	private slots:
		void TimerDone();
		void StartTimerSlot(int timeout);		
		void StopTimerSlot();		
	private:
		bool m_isActive;
		QTimer m_timer;
		QFile  m_file;
	};
};

#endif
