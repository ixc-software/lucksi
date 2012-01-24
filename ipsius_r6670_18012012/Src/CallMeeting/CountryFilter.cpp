#include "stdafx.h"
#include "Sip/undeferrno.h"
#include "CountryFilter.h"
#include "NObjCallMeeting.h"
#include "QueuedMeetingCall.h"
#include "DRI/commanddriresult.h"
#include "resip/stack/SdpContents.hxx"
#include "resip/stack/SipMessage.hxx"
#include "iSip/SipUtils.h"
#include "iSip/SipMessageHelper.h"

namespace CallMeeting
{
	using namespace resip;

	CountryFilter::CountryFilter(NObjCallMeeting &owner, 
		Utils::SafeRef<iLogW::ILogSessionCreator> logCreator,
		const QString &country,
		const QString &code,
		int maxCalls) :
		iCore::MsgObject(owner.getMsgThread()),
		m_owner(owner),
		m_country(country),
		m_maxCalls(maxCalls)
	{
		m_log.reset(logCreator->CreateSession(QString("CF_%1").arg(m_country).toStdString(), true));
		m_infoTag = m_log->RegisterRecordKind(L"Info", true);

		AddCode(code);
	}

	// -------------------------------------------------------------------------------

	CountryFilter::~CountryFilter()
	{
	}

	// -------------------------------------------------------------------------------

	const QString &CountryFilter::Country() const
	{
		return m_country;
	}

	// -------------------------------------------------------------------------------

	void CountryFilter::AddCode(const QString &code)
	{
		// !code.contains(QRegExp("(\\D)"))
		if (!code.isEmpty()) m_codes << code;  
	}

	// -------------------------------------------------------------------------------

	const QStringList &CountryFilter::Codes() const
	{
		return m_codes;
	}

	// -------------------------------------------------------------------------------

	void CountryFilter::StatAll(QStringList &res) const
	{
		res << QString("CountryFilter: %1; Max calls: %2").arg(m_log->NameStr().c_str()).arg(m_maxCalls);
		m_stat.ToString(res);
	}

	// -------------------------------------------------------------------------------

	void CountryFilter::StatShort(Utils::TableOutput &table) const
	{
		m_stat.ShortStat(m_country, table);
	}

	// -------------------------------------------------------------------------------

	void CountryFilter::RestartStat()
	{
		m_stat.Restart();
	}

	// -------------------------------------------------------------------------------

	void CountryFilter::Process()
	{
		m_queuedCalls.Process();
		m_readyCalls.Process();
		m_processedCalls.Process();
	}

	// -------------------------------------------------------------------------------

	bool CountryFilter::AcceptNewCall(const QString &number, 
		boost::shared_ptr<CallMeetingStatList> stat)
	{
		if(m_codes.isEmpty()) return false;

		for(int i = 0; i < m_codes.size(); ++i)
		{
			if(number.startsWith(m_codes[i])) 
			{
				stat->Push(m_stat);
				return true; 
			}
		}

		return false;
	}

	// -------------------------------------------------------------------------------

	void CountryFilter::IncomingCall(Utils::SafeRef<iSip::IRemoteParticipant> callSip, 
		const resip::SipMessage &msg,
		boost::shared_ptr<CallMeetingStatList> stat)
	{
		iSip::SipMessageHelper msgHelper(msg);

		std::string destination(msgHelper.To().uri().user());


		if(m_maxCalls != 0 && m_stat.CurrentCalls() == m_maxCalls)
		{
			if(m_log->LogActive(m_infoTag)) 
			{
				*m_log << m_infoTag << "Max calls on this point. " 
					<< iSip::SipUtils::ToString(iSip::SipUtils::Short, msg) 
					<< iLogW::EndRecord;
			}
			m_stat.IncDestinationBusy();
			callSip->DestroyParticipant(480);
			return;
		}

		if(Profile().RejectIfDstBusy() && 
			(m_queuedCalls.CalledAddrIsExist(destination) || 
			m_readyCalls.CalledAddrIsExist(destination) ||
			m_processedCalls.CalledAddrIsExist(destination)))
		{
			if(m_log->LogActive(m_infoTag)) 
			{
				*m_log << m_infoTag << "Destination busy. "
					<< iSip::SipUtils::ToString(iSip::SipUtils::Short, msg) 
					<< iLogW::EndRecord;
			}

			m_stat.IncDestinationBusy();
			callSip->DestroyParticipant(480);
			return;
		}

		stat->Push(m_stat);

		m_queuedCalls.AddCall(new QueuedCall(*this, *m_log, stat, callSip, destination));
		return;
	}

	// -------------------------------------------------------------------------------

	CountryFilter::QueuedCall *CountryFilter::AllocCallReadyForOtherFilter()
	{
		QueuedCall *call = m_readyCalls.AllocCallReadyForOtherFilter();
		if(call != 0) m_processedCalls.AddCall(call);
		return call;
	}

	// -------------------------------------------------------------------------------

	const CallMeetingProfile &CountryFilter::Profile() const
	{
		return m_owner.Profile();
	}

	// -------------------------------------------------------------------------------

	bool CountryFilter::IsReadyCallExist() const
	{
		return !m_readyCalls.IsEmpty();
	}

	// -------------------------------------------------------------------------------

	bool CountryFilter::ListOfCalls::IsEmpty() const
	{
		return m_listCalls.empty();
	}

	// -------------------------------------------------------------------------------

	CountryFilter::QueuedCall &CountryFilter::ListOfCalls::Front()
	{
		ESS_ASSERT(!IsEmpty());

		return m_listCalls.front();
	}

	// -------------------------------------------------------------------------------

	void CountryFilter::ListOfCalls::AddCall(QueuedCall *call)
	{
		ESS_ASSERT(call != 0);

		m_listCalls.push_back(call);
	}

	// -------------------------------------------------------------------------------

	CountryFilter::QueuedCall *CountryFilter::ListOfCalls::
		DeleteCall(const QueuedCall *call)
	{
		ESS_ASSERT(call != 0);

		for(List::iterator i = m_listCalls.begin();
			i != m_listCalls.end();
			++i)
		{
			if (&(*i) == call) return m_listCalls.release(i).release();
		}
		return 0;
	}

	// -------------------------------------------------------------------------------

	bool CountryFilter::ListOfCalls::CalledAddrIsExist(const std::string &val)
	{
		for(List::const_iterator i = m_listCalls.begin();
			i != m_listCalls.end(); ++i)
		{
			if(i->CalledAddr() == val) return true;
		}
		return false;
	}

	// -------------------------------------------------------------------------------

	CountryFilter::QueuedCall *CountryFilter::ListOfCalls::
		AllocCallReadyForOtherFilter()
	{
		for(List::iterator i = m_listCalls.begin();
			i != m_listCalls.end(); ++i)
		{
			if(i->IsReadyForOtherFilter()) return m_listCalls.release(i).release();
		}
		return 0;
	}

	// -------------------------------------------------------------------------------

	void CountryFilter::ListOfCalls::Process()
	{
		for(List::iterator i = m_listCalls.begin(); i != m_listCalls.end(); ++i)
		{
			i->Process();
		}
	}

	// -------------------------------------------------------------------------------

	void CountryFilter::Ready(QueuedCall &call)
	{
		PutMsg(this, &T::onReady, &call, true);
	}

	// -------------------------------------------------------------------------------

	void CountryFilter::onReady(QueuedCall *call)
	{
		ESS_ASSERT(call != 0);
		if (!call->IsActive()) return;

		ESS_ASSERT(m_queuedCalls.DeleteCall(call) == call);

		QueuedCall *readyCall = 0;
		while(!m_readyCalls.IsEmpty())
		{
			readyCall = m_readyCalls.DeleteCall(&m_readyCalls.Front());
			m_processedCalls.AddCall(readyCall);
			if (readyCall->IsActive()) break;
			readyCall = 0;
		}
		if (readyCall == 0)
		{
			m_readyCalls.AddCall(call);
			return;
		}

		m_processedCalls.AddCall(call);
		QueuedCall::redirectToParticipant(*readyCall, *call, Profile().UseRedirect());
	}

	// -------------------------------------------------------------------------------

	void CountryFilter::ReadyForOtherFilter(QueuedCall &call)
	{
		PutMsg(this, &T::onReadyForOtherFilter, &call, true);
	}

	// -------------------------------------------------------------------------------

	void CountryFilter::onReadyForOtherFilter(QueuedCall *call)
	{
		ESS_ASSERT(call != 0);
		if (!call->IsActive() || !call->IsReadyForOtherFilter()) return;

		QueuedCall *readyCall = m_owner.AllocCallReadyForOtherFilter(*this);

		if(readyCall == 0) return;
			
		ESS_ASSERT(call == m_readyCalls.DeleteCall(call) );	
		m_processedCalls.AddCall(call);
		QueuedCall::redirectToParticipant(*readyCall, *call, Profile().UseRedirect());
	}

	// -------------------------------------------------------------------------------

	void CountryFilter::DeleteCall(QueuedCall &call)
	{
		PutMsg(this, &T::onDelete, &call);		
	}

	// -------------------------------------------------------------------------------

	void CountryFilter::onDelete(QueuedCall *call)
	{
		ESS_ASSERT(m_queuedCalls.DeleteCall(call) != 0 ||
			m_readyCalls.DeleteCall(call) != 0||
			m_processedCalls.DeleteCall(call) != 0);
	} 
};


