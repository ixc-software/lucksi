#pragma once 

#include "stdafx.h"
#include "CallMeetingStat.h"
#include "iCore/MsgObject.h"
#include "iLog/LogWrapper.h"
#include "Utils/HostInf.h"
#include "CallMeetingProfile.h"

namespace iSip  {	class IRemoteParticipant;	}
namespace resip {	class SipMessage;	}

namespace Utils	{	class TableOutput;	}

namespace CallMeeting
{
	class NObjCallMeeting;

	class CountryFilter : public iCore::MsgObject
	{
		typedef CountryFilter T;
	public:
		class QueuedCall;

		CountryFilter(NObjCallMeeting &owner,
			Utils::SafeRef<iLogW::ILogSessionCreator> logCreator,
			const QString &country,
			const QString &code,
			int maxCalls = 0);

		~CountryFilter();
		const QString &Country() const;

		void AddCode(const QString &code);
		const QStringList &Codes() const;

		void StatAll(QStringList &res) const;
		void StatShort(Utils::TableOutput &table) const;		
		void RestartStat();

		void Process();

		bool AcceptNewCall(const QString &number, 
			boost::shared_ptr<CallMeetingStatList> stat);

		void IncomingCall(Utils::SafeRef<iSip::IRemoteParticipant> call, 
			const resip::SipMessage &msg,
			boost::shared_ptr<CallMeetingStatList> stat);

		QueuedCall *AllocCallReadyForOtherFilter();

	private:
		const CallMeetingProfile &Profile() const;
		bool IsReadyCallExist() const;

	private:        
		class ListOfCalls : boost::noncopyable
		{
		public:
			bool IsEmpty() const;
			QueuedCall &Front();
			void AddCall(QueuedCall *);
			QueuedCall *DeleteCall(const QueuedCall *);
			bool CalledAddrIsExist(const std::string &val);
			QueuedCall *AllocCallReadyForOtherFilter();
			void Process();
		private:
			typedef boost::ptr_list<QueuedCall> List;
			List m_listCalls; 
		};

		void Ready(QueuedCall &);
		void onReady(QueuedCall *call);
		void ReadyForOtherFilter(QueuedCall &);
		void onReadyForOtherFilter(QueuedCall *call);
		void DeleteCall(QueuedCall &);
		void onDelete(QueuedCall *);

	private: 
		NObjCallMeeting &m_owner;

		QString m_country;
		QStringList m_codes;
		int m_maxCalls;
		boost::scoped_ptr<iLogW::LogSession>  m_log;
		iLogW::LogRecordTag m_infoTag;
		CallMeetingStat m_stat;
		ListOfCalls m_queuedCalls;
		ListOfCalls m_readyCalls;
		ListOfCalls m_processedCalls;
	};
};


