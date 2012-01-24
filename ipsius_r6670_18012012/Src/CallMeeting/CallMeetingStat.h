#pragma once 

#include "stdafx.h" 
#include "Platform/Platform.h" 
#include "Utils/StatisticElement.h" 

namespace Utils	{	class TableOutput;	}

namespace CallMeeting
{
    class CallMeetingStat : boost::noncopyable
    {
	public:

		CallMeetingStat();
		void Restart();
		
		void ToTable(Utils::TableOutput &table) const;
		void ToString(QStringList &out) const;

		static void HeadShortStat(Utils::TableOutput &table);
		void TimerHeader(QStringList &out) const;
		void CallHeader(QStringList &out) const;

		void ShortStat(const QString &name, Utils::TableOutput &table) const;
		int TotalCalls() const;
		void IncCurrentCalls();
		void DecCurrentCalls();	
		int  CurrentCalls() const;
		void IncCurrentTalks();
		void DecCurrentTalks();		
		void IncDestinationBusy();
		void IncMaxTalkDuration();		
		void IncRelByTimeout();		
		void IncCanceledCalls();

		void AddWaitAnswerDuration(int timeout);
		void AddCancelDuration(int timeout);
		void AddTalkDuration(int timeout);
	private:
		QDateTime m_activation;
		QDateTime m_lastCall;
		QDateTime m_lastTalk;
		int m_totalCalls;
		int m_totalTalks;
		int m_currentCalls;
		int m_currentTalks;
		int m_destinationBusy;
		int m_maxTalkDuration;
		int m_relByTimeout;
		int m_canceledCalls;
		Utils::StatElementForInt m_talkDuration;
		Utils::StatElementForInt m_waitingDuration;
		Utils::StatElementForInt m_cancelDuration;
	};

	// -------------------------------------------------------------------------------

	class CallMeetingStatList : boost::noncopyable
	{
		typedef CallMeetingStat T;
	public:
		~CallMeetingStatList()
		{
			Run(&T::DecCurrentCalls);
		}

		void Push(CallMeetingStat &item)
		{
			item.IncCurrentCalls();
			m_list.push_back(&item);
		}

		void IncCurrentTalks()	{	Run(&T::IncCurrentTalks); };
		void DecCurrentTalks()	{	Run(&T::DecCurrentTalks); };
		void IncDestinationBusy()	{	Run(&T::IncDestinationBusy); };
		void IncMaxTalkDuration()	{	Run(&T::IncMaxTalkDuration); };
		void IncRelByTimeout()	{	Run(&T::IncRelByTimeout); };
		void IncCanceledCalls()	{	Run(&T::IncCanceledCalls); };

		void AddWaitAnswerDuration(int timeout)	{	Run(&T::AddWaitAnswerDuration, timeout); };
		void AddCancelDuration(int timeout)	{	Run(&T::AddCancelDuration, timeout); };
		void AddTalkDuration(int timeout)	{	Run(&T::AddTalkDuration, timeout); };
	private:
		typedef std::vector<CallMeetingStat*> List;
		List m_list;

		template <typename PAction, typename TPar>
			void Run(PAction action, const TPar &par)
		{
			for(List::iterator i = m_list.begin(); i != m_list.end(); ++i)
			{
				((*i)->*action)(par);
			}
		}
		typedef void (T::*SAction)();
		void Run(SAction action)
		{
			for(List::iterator i = m_list.begin(); i != m_list.end(); ++i)
			{
				((*i)->*action)();
			}
		}
	};
};	


