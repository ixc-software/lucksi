#ifndef _STATISTIC_SIP_H_
#define _STATISTIC_SIP_H_

#include "stdafx.h" 
#include "Utils/SafeRef.h"
#include "Utils/OutputFormatters.h"

namespace Calls 
{
    class StatisticCalls : boost::noncopyable
    {
        class Direction : boost::noncopyable, public virtual Utils::SafeRefServer
        {
			friend class StatisticCalls;

			// Класс статистики вызовов одного направления
            int m_currentCalls;
            int m_allCalls;
            int m_currentTalks;
            int m_allTalks;
			
			void Restart();

        public:
            typedef Utils::SafeRef<Direction> SafeRef;
            Direction();
			void ToTable(const QString &dir, 
				Utils::TableOutput &table) const;
			void Add(const Direction &);
            void Add();
            void Dec();
            void AddTalk();
            void DecTalk();
        };

    // ------------------------------------------
    // Call
	public:
		class Call
		{
		public:
			Call(Direction::SafeRef statistic);
			~Call();
			void Talk();
		private:
			bool m_talk;
			Direction::SafeRef m_statistic;    
		};

	// methods
	public:
		StatisticCalls();
		Direction::SafeRef OutgoingCall() { return &m_outgoing; }
		Direction::SafeRef IncomingCall() { return &m_incoming; }

		void Restart();
		QStringList ToTable() const;	
	// member
	private:
		Direction  m_outgoing;
		Direction  m_incoming;

		QDateTime m_activation;
	};
};

#endif
