#include "stdafx.h"

#include "NObjDss1ToSip.h"
#include "Dss1ToSipCall.h"
#include "NObjDss1.h"
#include "NObjDss1Interface.h"
#include "NObjDss1CallProfile.h"
#include "iSip/NObjSipOutCallProfile.h"
#include "iSip/NObjSipAccessPoint.h"
#include "iSip/SipUtils.h"
#include "iSip/SipMessageHelper.h"
#include "iReg/ISysReg.h"

namespace 
{
	std::string DigitsAddrToSipUri(const std::string &number, 
		const Utils::HostInf &host)
	{
		std::ostringstream out;
		out << "sip:" << number << "@" << host.Address();

		if(host.Port() != 5060 && host.Port() != -1) out << ":" << host.Port();

		return out.str();
	}

	Utils::QtEnumResolvHandler<ISDN::IeConstants, ISDN::IeConstants::SideType> GSideTypeResolver;
};

namespace Dss1ToSip
{
	NObjDss1ToSip::NObjDss1ToSip(Domain::IDomain *pDomain, const Domain::ObjectName &name) : 
		Domain::NamedObject(pDomain, name),
		m_outCallProfile(new iSip::NObjSipOutCallProfile(this, "ToSip")),
		m_inCallProfile (new NObjDss1InCallProfile (this, "FromSip")),
		m_isEnabled(false),
		m_waitActiveTimer(this, &T::OnWaitActive),
		m_activeTrunkCount(0)
	{
		m_numberingPlan = NumberingPlan::ISDN;
		m_typeOfNumber = TypeOfNumber::Subscriber;
	}

	// -------------------------------------------------------------------------------

	NObjDss1ToSip::~NObjDss1ToSip()
	{
		if (IsLogActive(InfoTag)) Log(InfoTag) << "Deleted." << iLogW::EndRecord;
	}

	// -------------------------------------------------------------------------------

	void NObjDss1ToSip::AddDss1(const QString &harrdType, const QString &name, const QString &boardAlias, 
		const QString &sideType,
		int intfNumber, int DChannelSlot)
	{
		bool ret = false;
		HardType::Value hardware = Dss1Def::Resolve<HardType>(harrdType.toStdString().c_str(), ret);
			
//		Utils::QtEnumResolver<Dss1Def, HardType::Value>().Resolve(harrdType.toStdString().c_str(), &ret);
		if (!ret) ThrowRuntimeException("Wrong hardware type. Allowed 'T1' or 'E1'");
		
		ISDN::IeConstants::SideType side = 
			GSideTypeResolver.Impl().Resolve(sideType.toStdString().c_str(), &ret);

		if (!ret) ThrowRuntimeException("Wrong side type. Allowed 'NT' or 'TE'");

		m_dss1.push_back(new NObjDss1(this, 
			name, 
			*this,
			hardware,
			side, 
			boardAlias, intfNumber, DChannelSlot));
	}

	// -------------------------------------------------------------------------------

	void NObjDss1ToSip::DeleteDss1(const QString &name)
	{
		ListDss1Link::iterator i = m_dss1.begin();
		while (i != m_dss1.end())
		{
			ESS_ASSERT(*i != 0);
			if((*i)->Name().Name() == name) break;
			++i;
		}
		if(i == m_dss1.end()) ThrowRuntimeException("Dss1 link '" + name.toStdString() + "' not found!");

		if(m_lastDss1Trunk.IsEqualIntf(*i)) m_lastDss1Trunk.Clear();
		(*i)->Stop();
		delete *i;
		m_dss1.erase(i);
	}

	// ---------------------------------------------------------------------

	void NObjDss1ToSip::Calls(DRI::ICmdOutput *pOutput, bool brief)
	{
		QStringList out;
		std::for_each(m_dss1.begin(), m_dss1.end(), boost::bind(&IBoardInfoReq::CallsInfoReq, _1, boost::ref(out), brief));
		pOutput->Add(out);
	}


	// ---------------------------------------------------------------------

	void NObjDss1ToSip::Info(DRI::ICmdOutput *pOutput)
	{
		BoardInfo::List list;
		std::for_each(m_dss1.begin(), m_dss1.end(), boost::bind(&IBoardInfoReq::BoardInfoReq, _1, boost::ref(list)));
		pOutput->Add(BoardInfo::ToString(list));
	}

	// -------------------------------------------------------------------------------

	void NObjDss1ToSip::Stat(DRI::ICmdOutput *pOutput)
	{
		pOutput->Add(m_stat.ToTable());
	}

	// -------------------------------------------------------------------------------

	void NObjDss1ToSip::RestartStat()
	{
		m_stat.Restart();
	}

	// ------------------------------------------------------------	

	void NObjDss1ToSip::WaitWhileNoactive(DRI::IAsyncCmd *pAsyncCmd, int maxTimeout)
	{
		AsyncBegin(pAsyncCmd);

		if(m_activeTrunkCount == 0)
			m_waitActiveTimer.Start(maxTimeout);
		else
			AsyncComplete(true);
	}

	// ----------------------------------------------------------------------
	// iSip::ISipCallManagerEvents
	
	void NObjDss1ToSip::OnIncomingParticipant(const iSip::ISipAccessPoint *src,
		Utils::SafeRef<iSip::IRemoteParticipant> callSip,
		const resip::SipMessage &msg,
		bool autoAnswer, 
		const iSip::ConversationProfile &conversationProfile)
	{
		ESS_ASSERT(m_sipPoint.IsEqualIntf(src));

		iSip::SipMessageHelper msgHelper(msg);

		std::string calledAddr(msgHelper.To().uri().user());

		std::string callingAddr( ( m_inCallProfile->GetCallingAddrType() == NObjDss1InCallProfile::From)  ?
			msgHelper.From().uri().user() :
			msgHelper.Contact().uri().user());

		bool isCorrectNumbers = ISDN::Dss1Digits::IsCorrectCharset(calledAddr);
		if(isCorrectNumbers && !ISDN::Dss1Digits::IsCorrectCharset(callingAddr))
		{
			if (m_inCallProfile->GetCallingAddrNoDigit() == NObjDss1InCallProfile::ReleaseCall)    
			{
				isCorrectNumbers = false;
			}
			
			if (m_inCallProfile->GetCallingAddrNoDigit() == NObjDss1InCallProfile::UsePilotNumber) 
			{
				callingAddr = m_inCallProfile->GetPilotNumber();
			}
		}

		if(!isCorrectNumbers)
		{
			if (IsLogActive(InfoTag))
			{
				Log(InfoTag) << "Incoming call. Incorrect number. " 
					<< iSip::SipUtils::ToString(iSip::SipUtils::Short, msg) 
					<< iLogW::EndRecord;
			}
			callSip->DestroyParticipant(480);
			return;
		}

		NObjDss1 *dss1Link = AllocDss1Link();
		if(dss1Link == 0)
		{
			if (IsLogActive(InfoTag))
			{
				Log(InfoTag) << "Incoming call. All channels busy."<< iLogW::EndRecord;
			}

			callSip->DestroyParticipant(480);
			return;
		}

		new Dss1ToSipCall(
			boost::bind(&T::CallRegistrator, this, _1, _2),
			m_stat,
			Log(),
			callSip, 
			*dss1Link, ISDN::DssPhoneNumber(calledAddr, 
			ISDN::IeConstants::Subscriber, ISDN::IeConstants::ISDNtelephony),
			ISDN::DssPhoneNumber(callingAddr, 
			ISDN::IeConstants::Subscriber, ISDN::IeConstants::ISDNtelephony));
	}

	// ----------------------------------------------------------------------

	void NObjDss1ToSip::OnRequestOutgoingParticipant(const iSip::ISipAccessPoint *src,
		Utils::SafeRef<iSip::IRemoteParticipant> newParticipant, 
		const resip::SipMessage &msg, 
		const iSip::ConversationProfile &conversationProfile)
	{
		ESS_UNIMPLEMENTED;
	}

	// ----------------------------------------------------------------------
	// impl IGateDss1Events

	void NObjDss1ToSip::Activated(const IGateDss1 *id) 
	{
		++m_activeTrunkCount;
		if (IsLogActive(InfoTag)) Log(InfoTag) << "Activated. Activate trunk count:" << m_activeTrunkCount << iLogW::EndRecord;
		if(AsyncActive())
		{
			m_waitActiveTimer.Stop();
			AsyncComplete(true);
			return;
		}
	}

	// ----------------------------------------------------------------------

	void NObjDss1ToSip::Deactivated(const IGateDss1 *id)
	{
		--m_activeTrunkCount;
		if (IsLogActive(InfoTag)) Log(InfoTag) << "Deactivated. Activate trunk count:" << m_activeTrunkCount << iLogW::EndRecord;
	}

	// ----------------------------------------------------------------------

	void NObjDss1ToSip::IncomingCall(const IGateDss1 *id,
		Utils::SafeRef<IDss1Call> callDss1,
		const ISDN::DssPhoneNumber &calledAddress,
		const ISDN::DssPhoneNumber &callingAddress)
	{
		if(m_sipPoint.IsEmpty())
		{
			callDss1->Release(0, 21);
			return;
		}
		
		// Поступил входящий вызов по Dss1

		boost::shared_ptr<iSip::IRemoteParticipantCreator::Parameters> 
			parameters(new iSip::IRemoteParticipantCreator::Parameters());

		parameters->SetDestinationUserName(calledAddress.Digits().getAsString().c_str());

		parameters->SourceUserName = callingAddress.Digits().getAsString().c_str();

		m_outCallProfile->UpdateCallParameters(*parameters);

		boost::shared_ptr<iReg::CallRecordWrapper> callRecord(
			new iReg::CallRecordWrapper(boost::bind(&iReg::ISysReg::CallInfo, &getDomain().SysReg(), Name(), _1)));

		new Dss1ToSipCall(
			boost::bind(&T::CallRegistrator, this, _1, _2),
			m_stat,
			Log(),
			callDss1, 
			callRecord,
			m_sipPoint,
			parameters);
	}

	// -------------------------------------------------------------------------------
	// property

	void NObjDss1ToSip::Enable()
	{
		if(m_isEnabled) return;
		
		if (m_sipPoint.IsEmpty()) ThrowRuntimeException("Sip access point isn't specified.");

		CheckCorrectProperty();
		
		if (IsLogActive(InfoTag)) Log(InfoTag) << "Enabled." << iLogW::EndRecord;
		
		std::for_each(m_dss1.begin(), m_dss1.end(), boost::bind(&NObjDss1::Start, _1));

		m_isEnabled = true;
	}

	// -------------------------------------------------------------------------------

	void NObjDss1ToSip::Disable()
	{
		if(!m_isEnabled) ThrowRuntimeException("Already disabled.");

		if (IsLogActive(InfoTag)) Log(InfoTag) << "Disabled." << iLogW::EndRecord;

		std::for_each(m_dss1.begin(), m_dss1.end(), boost::bind(&NObjDss1::Stop, _1));

		m_isEnabled = false;
	}

	// -------------------------------------------------------------------------------------

	void NObjDss1ToSip::SipPoint(const QString &val)
	{
		if (val.isEmpty())
		{
			m_sipPointName = val;
			m_sipPoint.Clear();
			return;
		}

		iSip::NObjSipAccessPoint *accessPoint = dynamic_cast<iSip::NObjSipAccessPoint*>
			(getDomain().FindFromRoot(Domain::ObjectName(val)));

		if (accessPoint == 0) ThrowRuntimeException("NObjSipAccessPoint named '" + val + "' not found");

		m_sipPointName = val;

		m_sipPoint = accessPoint->AccessPoint();

		m_sipPoint->SetCallReceiver(
			Name().GetShortName().toStdString(),	
			m_selfHost.Create<iSip::ISipAccessPointEvents&>(*this));

	}


	// -------------------------------------------------------------------------------

	void NObjDss1ToSip::CheckCorrectProperty()
	{
		m_inCallProfile->CheckCorrectProperty(); 
	}
	
	// property 
	// ------------------------------------------------------------

	void NObjDss1ToSip::CallRegistrator(Dss1ToSipCall *call, bool addToList)
	{
		if (addToList) 
		{
			m_calls.push_back(call);
			return;
		}


		ListOfCalls::iterator i = m_calls.begin();
		while(i != m_calls.end())
		{
			if (&(*i) == call)
				break;
			++i;
		}
		ESS_ASSERT(i != m_calls.end());
		PutMsg(this, &T::OnDeleteCall, m_calls.release(i).release());
	}

	// -------------------------------------------------------------------------------

	void NObjDss1ToSip::OnWaitActive(iCore::MsgTimer *pT)
	{

		if(AsyncActive()) AsyncComplete(false, "Dss1 isn't active");
	}

	// ----------------------------------------------------------------------

	NObjDss1* NObjDss1ToSip::AllocDss1Link()
	{
		NObjDss1 *prev = 0;
		NObjDss1 *next = 0;
		bool afterLastLink = false;

		for(ListDss1Link::iterator i = m_dss1.begin();
			i != m_dss1.end(); ++i)
		{
			if(m_lastDss1Trunk.IsEqualIntf(*i))
			{
				afterLastLink = true;
				continue;
			}
			if(!(*i)->isDss1Active()) continue;
			
			if(afterLastLink)
			{
				next = *i;
				break;
			}
			prev = *i;
		}
		
		if(next != 0)
			m_lastDss1Trunk = Utils::SafeRef<NObjDss1>(next);
		else if(prev != 0)
			m_lastDss1Trunk = Utils::SafeRef<NObjDss1>(prev);
		else if(m_lastDss1Trunk.IsEmpty() || !m_lastDss1Trunk->isDss1Active()) 
			return 0;

		return &(*m_lastDss1Trunk);
	}

}


