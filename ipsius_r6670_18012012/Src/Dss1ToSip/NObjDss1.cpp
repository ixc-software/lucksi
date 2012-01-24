#include "stdafx.h"

#include "NObjDss1.h"
#include "NObjL1DriverForGate.h"
#include "NObjDss1Interface.h"
#include "Dss1CallForGate.h"
#include "InfraGateDss1.h"
#include "ISDN/IsdnStack.h"
#include "ISDN/L2Profiles.h"
#include "ISDN/NObjL3Profile.h"
#include "iLog/LogManager.h"
#include "Utils/FullEnumResolver.h"
#include "Utils/ExeName.h"
#include "iReg/ISysReg.h"

namespace Dss1ToSip
{
	class NObjDss1::CallProfile 
	{
	public:
		static void Init(NObjDss1 &owner, Dss1CallForGate::Profile &profile)
		{
			profile.m_owner = &owner;
			profile.m_infra = owner.m_infra.get();
			profile.m_stat = &owner.m_stat;
			profile.m_addressComplete = owner.m_addressCompleteSize;
			profile.m_sendAlertingTone = owner.m_sendAlertingTone;
			profile.m_logCreator = owner.Log().LogCreator();
		}
	};
	class NObjDss1::BoardOrderStrategy : boost::noncopyable
	{
	public:
		BoardOrderStrategy(NObjDss1 &owner) : m_owner(owner),
			m_currentBoard(-1){}

		NObjDss1Interface *AllocBoard()
		{
			NObjDss1Interface *board = m_owner.FindNextBoard(m_currentBoard);
			m_currentBoard = board->InterfaceNumber();
			return board;
		}
	private:
		NObjDss1 &m_owner;
		int m_currentBoard;
	};


};

namespace Dss1ToSip
{
	NObjDss1::NObjDss1(Domain::NamedObject *pParent, const Domain::ObjectName &name,
		IGateDss1Events &user,
		HardType::Value hardType,
		ISDN::IeConstants::SideType sideType, 
		const QString &boardAlias, int intfNumber, int DChannelSlot) : 
		NamedObject(&pParent->getDomain(), name, pParent),
		m_isEnabled(false),
		m_isActivedByOwner(false),
		m_state(NotActive),
		m_activationCount(0),
		m_hardType(hardType),
		m_sigInterface(intfNumber),
		m_useSharpAsNumberComplete(false),
		m_delSharpFromNumber(true),
		m_addressCompleteSize(0),
		m_sendAlertingTone(true),
		m_relAnonymCall(false),
		m_restartReq(false),
		m_l2traceInd(false),
		m_user(user),
		m_l3Profile(new ISDN::NObjL3Profile(this, "L3", sideType)),
		m_driverL1(new NObjL1DriverForGate(*this, "L1")),
		m_server(getDomain(), this),
		m_infra(new InfraGateDss1(getDomain(), getDomain().Log()))
	{
		if(DChannelSlot == Dss1Def::CDChannelNPos) ThrowRuntimeException("DChannel slot is undefined.");

		NObjDss1Interface *board = new NObjDss1Interface(*this, 
			boardAlias, m_hardType, intfNumber, DChannelSlot);

		Enable(true);
		m_waitAnswerTimeout = 0;
		m_boards.push_back(board);
	}

	// -------------------------------------------------------------------------------

	NObjDss1::~NObjDss1()
	{
		RegistratorLog("Deleted.");
	}

	// -------------------------------------------------------------------------------

	void NObjDss1::Start()
	{
		ESS_ASSERT(!m_isActivedByOwner);

		m_isActivedByOwner = true;

		if(m_isEnabled) StartImpl();
	}

	// -------------------------------------------------------------------------------

	void NObjDss1::Stop()
	{
		ESS_ASSERT(m_isActivedByOwner);

		m_isActivedByOwner = false;

		StopImpl();
	}

	// -------------------------------------------------------------------------------

	void NObjDss1::AddInterface(const QString &boardAlias, int intfNumber)
	{
		ListBoards::iterator next = m_boards.begin();
		for(ListBoards::iterator i = m_boards.begin();
			i != m_boards.end(); ++i)
		{
			if ((*i)->InterfaceNumber() == intfNumber) ThrowRuntimeException("Board already created.");			
			if ((*i)->NameBoard() == boardAlias) ThrowRuntimeException("Alias already used.");

			if ((*i)->InterfaceNumber() > intfNumber)  next = i;	
		}

		NObjDss1Interface *board = new NObjDss1Interface(*this, boardAlias, m_hardType, intfNumber);
		
		m_boards.insert(next, board);
	}

	// -------------------------------------------------------------------------------

	void NObjDss1::DeleteInterface(const QString &boardAlias)
	{	
		ListBoards::iterator i = std::find_if(m_boards.begin(), m_boards.end(), 
			(boost::bind(&NObjDss1Interface::NameBoard, _1)) == boardAlias);

		if(i == m_boards.end()) ThrowRuntimeException("Board not found.");

		if(IsStarted() && (*i)->InterfaceNumber() == SigInterface()) 
		{
			ThrowRuntimeException("Dss1 already started. Can't delete signaling interface.");
		}

		delete *i;
		m_boards.erase(i);	
	}

	// -------------------------------------------------------------------------------

	QString NObjDss1::GetStateStr() const
	{
		return	QString("%1 | %2 | %3").
			arg((m_isActivedByOwner) ? "ActivatedByOwner" : "DeactivatedByOwner"). 
			arg((m_isEnabled) ? "Enabled" : "Disabled").
			arg(Utils::EnumResolve(m_state).c_str());
	}

	// -------------------------------------------------------------------------------

	QString NObjDss1::GetShortStateStr() const
	{
		if(!m_isActivedByOwner) return "DeactivatedByOwner";
		if(!m_isEnabled) return "Disabled";

		return Utils::EnumResolve(m_state).c_str();
	}

	// -------------------------------------------------------------------------------

	void NObjDss1::SigInterface(int number)
	{
		if(IsStarted()) ThrowRuntimeException("Dss1 already started. Changing of DChannel location is not allowed.");

		NObjDss1Interface *interface = FindBoard(number);
		if (interface == 0)
		{
			std::ostringstream out;
			out << "Interface " << number << " is undefined!";
			ThrowRuntimeException(out.str());
		}

		if(interface->DChannel() == 0) 
		{
			std::ostringstream out;
			out << "DChannel slot is undefined in interface " <<  number;
			ThrowRuntimeException(out.str());
		}

		m_sigInterface = interface->InterfaceNumber();
	}

	// -------------------------------------------------------------------------------

	QString NObjDss1::LastActivation() const
	{
		return m_lastActivation.toString(Qt::SystemLocaleShortDate);
	}

	// -------------------------------------------------------------------------------

	QString NObjDss1::LastDeactivation() const
	{
		return m_lastDeactivation.toString(Qt::SystemLocaleShortDate);
	}

	// -------------------------------------------------------------------------------

	void NObjDss1::Calls(DRI::ICmdOutput *pOutput, bool brief)
	{
		QStringList out;
		CallsInfoReq(out, brief);
		pOutput->Add(out);
	}

	// -------------------------------------------------------------------------------

	void NObjDss1::Info(DRI::ICmdOutput *pOutput)
	{
		BoardInfo::List list;
		BoardInfoReq(list);
		pOutput->Add(BoardInfo::ToString(list));
	}

	// -------------------------------------------------------------------------------

	void NObjDss1::Stat(DRI::ICmdOutput *pOutput)
	{
		pOutput->Add(m_stat.ToTable());
	}

	// -------------------------------------------------------------------------------

	void NObjDss1::RestartStat()
	{
		m_stat.Restart();
	}

	// -------------------------------------------------------------------------------

	void NObjDss1::CallsDetail(QString fileName)
	{
		Utils::StringList sl;
		for(ListOfCalls::const_iterator i = m_calls.begin(); i != m_calls.end(); ++i)
		{
			const IDss1Call &call = *i;
			sl << call.CallName().c_str();
		}
		// make file name
		if (fileName.isEmpty()) fileName = "CallsDetail.txt";
		fileName = Utils::ExeName::GetExeDir().c_str() + fileName;
		// save
		sl.SaveToFile(fileName);
	}

	// -------------------------------------------------------------------------------

	void NObjDss1::L2Statistic(DRI::ICmdOutput *pOutput)
	{
		if(m_stackIsdn == 0) ThrowRuntimeException("Dss1 isn't active.");		

		pOutput->Add(m_stackIsdn->getL2Stat().getAsString().c_str());
	}

	// -------------------------------------------------------------------------------

	void NObjDss1::ClearL2Statistic()
	{
		if(m_stackIsdn == 0) ThrowRuntimeException("Dss1 isn't active.");		

		m_stackIsdn->ClearL2Stat();
	}

	// -------------------------------------------------------------------------------

	void NObjDss1::L3Statistic(DRI::ICmdOutput *pOutput)
	{
		if(m_stackIsdn == 0) ThrowRuntimeException("Dss1 isn't active.");		

		pOutput->Add(m_stackIsdn->getL3Stat().getAsString().c_str());
	}

	// -------------------------------------------------------------------------------

	void NObjDss1::ClearL3Statistic()
	{
		if(m_stackIsdn == 0) ThrowRuntimeException("Dss1 isn't active.");		

		m_stackIsdn->ClearL3Stat();
	}

	// -------------------------------------------------------------------------------

	void NObjDss1::L1Activated()
	{
		PutMsg(this, &T::OnActivateDss1Stack);
	}

	// -------------------------------------------------------------------------------

	void NObjDss1::L1Deactivated()
	{
		PutMsg(this, &T::OnDeactivateDss1Stack);
	}

	// -------------------------------------------------------------------------------

	Utils::SafeRef<IHardwareToL1> NObjDss1::HardwareToL1(int interfaceNumber)
	{
		return (interfaceNumber == SigInterface()) ? m_driverL1 : Utils::SafeRef<IHardwareToL1>();
	}

	// -------------------------------------------------------------------------------
	// IObjectLinksHost impl

	Utils::SafeRef<ObjLink::IObjectLinksServer> NObjDss1::getObjectLinksServer()
	{
		return m_server.getInterface();
	}

	// -------------------------------------------------------------------------------
	// ISDN::ICallBackDss

	void NObjDss1::Activated() 
	{      
		if (!m_restartReq)
		{
			SetState(Active, "L2 activated");
			m_user.Activated(this);
		}
		else
		{
			SetState(WaitRestartComplete, "L2 activated");
			ISDN::ILayerDss *l3 = m_stackIsdn->GetL3Ptr();
			l3->RestartReq();
		}
		++m_activationCount;
		m_lastActivation = QDateTime::currentDateTime();
	}

	// -------------------------------------------------------------------------------

	void NObjDss1::Deactivated() 
	{   
		SetState(NotActive, "Deactivated");
		m_state = NotActive;
		m_user.Deactivated(this);
		m_lastDeactivation = QDateTime::currentDateTime();
	}

	// -------------------------------------------------------------------------------

	void NObjDss1::IncomingCall(IncommingCallParametrs par) 
	{         
		if(par.m_bindToDssCall->IsEmpty()) return;

		Dss1CallForGate::Profile profile;
		CallProfile::Init(*this, profile);

		Dss1CallForGate *call = new Dss1CallForGate(profile, par);
		m_calls.push_back(call);
	}

	// -------------------------------------------------------------------------------

	void NObjDss1::DssErrorMsg(boost::shared_ptr<const ISDN::DssWarning> pWarning)
	{         
		if (IsLogActive(WarningTag)) 
			Log(WarningTag) << "Dss1 warning: " << pWarning->ToString() << iLogW::EndRecord;
	}

	// -------------------------------------------------------------------------------

	void NObjDss1::RestartComplete(CompleteInfo info)
	{
		SetState(Active, 
			(info.m_ok) ? 
				"Restart complete." : "Restart incomplete. " + info.m_addInfo);

		m_user.Activated(this);
	}

	// -------------------------------------------------------------------------------
	// IGateDss1

	Utils::SafeRef<IDss1Call> NObjDss1::RunOutCall(Utils::SafeRef<IDss1CallEvents> user,
		const ISDN::DssPhoneNumber &calledNum, 
		const ISDN::DssPhoneNumber &callingNum)
	{
		if(!isDss1Active()) return Utils::SafeRef<IDss1Call>();


		Dss1CallForGate::Profile profile;
		CallProfile ::Init(*this, profile);
		profile.m_waitAnswerTimeout = m_waitAnswerTimeout;

		boost::shared_ptr<ISDN::DssCallParams> par;

		if (m_useSharpAsNumberComplete) 
		{
			const std::string &str = calledNum.Digits().getAsString();
			
			if (!str.empty() && str.at(str.size() -1) != '#') 
			{
				ISDN::DssPhoneNumber calledNumTemp(calledNum);
				calledNumTemp.AppendDigits(ISDN::Dss1Digits("#"));
				par.reset(ISDN::DssOutCallParams::Create( 
					m_infra->IsdnInfra(), calledNumTemp, callingNum));
			}
		}

		if (par == 0)
		{
			par.reset(ISDN::DssOutCallParams::Create( 
				m_infra->IsdnInfra(), calledNum, callingNum));
		}


		Dss1CallForGate *call = new Dss1CallForGate(user, 
			profile, *m_stackIsdn->GetDssIntf(), par);

		m_calls.push_back(call);

		return call;
	}

	// -------------------------------------------------------------------------------

	bool NObjDss1::isDss1Active() const 
	{ 
		return m_state == Active; 
	}

	// -------------------------------------------------------------------------------
	// IGateDss1ForCall

	void NObjDss1::IncomingCall(const IGateDss1ForCall *id,
		Utils::SafeRef<IDss1Call> call,
		const ISDN::DssPhoneNumber &calledAddress, 
		const ISDN::DssPhoneNumber &callingAddress)
	{
		ESS_ASSERT(this == id);
		
		if(callingAddress.Digits().IsEmpty() && m_relAnonymCall)
		{
			call->Release(0, 21);
			return;
		}

		ISDN::DssPhoneNumber callingAddressTemp(callingAddress);

		if(callingAddressTemp.Digits().IsEmpty()) 
		{
			callingAddressTemp.setDigits(ISDN::Dss1Digits(m_anonymCalledAddr));
		}
		m_user.IncomingCall(this, call, calledAddress, callingAddressTemp);
	}

	// -------------------------------------------------------------------------------

	iCmpExt::ICmpChannelCreator& NObjDss1::CmpChannelCreator(int interfaceNumber)
	{
		NObjDss1Interface *board = FindBoard(interfaceNumber);
		ESS_ASSERT(board != 0);
		return *board;
	}

	// -------------------------------------------------------------------------------

	void NObjDss1::SaveCallInfo(int interfaceNumber,
		int channelNumber, Utils::WeakRef<GateCallInfo&> callInfo)
	{
		NObjDss1Interface *board = FindBoard(interfaceNumber);
		ESS_ASSERT(board != 0);
		board->SaveCallInfo(channelNumber, callInfo);
	}

	// -------------------------------------------------------------------------------

	QString NObjDss1::InterfaceName(int interfaceNumber) const
	{
		const NObjDss1Interface *board = FindBoard(interfaceNumber);
		ESS_ASSERT(board != 0);
		return board->Name().Name();
	}

	// -------------------------------------------------------------------------------

	void NObjDss1::DeleteCall(const IDss1Call *call)
	{
		ListOfCalls::iterator i = m_calls.begin();
		while (i != m_calls.end())
		{
			if(&(*i) == call) break;

			++i;
		}
		ESS_ASSERT(i != m_calls.end());

		PutMsg(this, &T::OnDeleteCall, m_calls.release(i).release());

	}

	// -------------------------------------------------------------------------------
	// IDssToGroup

	int NObjDss1::CountIntf() const
	{
		return m_boards.size();
	}

	// -------------------------------------------------------------------------------

	ISDN::SharedBChansWrapper NObjDss1::CaptureAny(int count, int interfaceNumber, bool isOutgoing)
	{
		if (interfaceNumber != -1)
		{
			NObjDss1Interface *board = FindBoard(interfaceNumber);
			ESS_ASSERT(board != 0);
			return board->CaptureAnyBChannelWrap(count, isOutgoing);
		}

		for(int i = 0; i < m_boards.size(); ++i)
		{
			NObjDss1Interface *board = m_boardStrategy->AllocBoard();
			ISDN::SharedBChansWrapper res(board->CaptureAnyBChannelWrap(count, isOutgoing));
			if(res != 0) return res;
		}
		return ISDN::SharedBChansWrapper();
	}

	// -------------------------------------------------------------------------------

	ISDN::SharedBChansWrapper NObjDss1::Capture(const ISDN::SetBCannels &inquredSet, 
		int interfaceNumber, bool exclusive, bool isOutgoing)
	{

		if(interfaceNumber == -1) interfaceNumber = m_sigInterface;
		
		NObjDss1Interface *board = FindBoard(interfaceNumber);

		if(board == 0) return ISDN::SharedBChansWrapper();
		
		ISDN::SharedBChansWrapper res = board->CaptureBChannelWrap(inquredSet, isOutgoing);
		
		if(res != 0) return res;

		if(exclusive) return ISDN::SharedBChansWrapper();

		return board->CaptureAnyBChannelWrap(inquredSet.Count(), isOutgoing);
	}

	// -------------------------------------------------------------------------------
	// IBoardInfoReq

	void NObjDss1::CallsInfoReq(QStringList &info, bool brief) const
	{
		std::for_each(m_boards.begin(), m_boards.end(), 
			boost::bind(&IBoardInfoReq::CallsInfoReq, _1, boost::ref(info), brief));
	}

	// -------------------------------------------------------------------------------

	void NObjDss1::BoardInfoReq(BoardInfo::List &info) const
	{
		std::for_each(m_boards.begin(), m_boards.end(), 
 			boost::bind(&IBoardInfoReq::BoardInfoReq, _1, boost::ref(info)));
	}

	// -------------------------------------------------------------------------------
	// property 

	void NObjDss1::Enable(bool par)
	{
		if(par)
		{
			if(m_isEnabled) ThrowRuntimeException("Already enabled.");

			m_isEnabled = true;

			if(m_isActivedByOwner) StartImpl();

			return;
		}

		if(!m_isEnabled) ThrowRuntimeException("Already disabled.");

		m_isEnabled = false;

		StopImpl();	
	}

	// -------------------------------------------------------------------------------

	QString	NObjDss1::HardwareType() const
	{
		return Dss1Def::ToString<HardType>(m_hardType).c_str();
	}

	// -------------------------------------------------------------------------------

	bool NObjDss1::L2TraceInd() const
	{	
		return m_l2traceInd; 
	}

	// -------------------------------------------------------------------------------

	void NObjDss1::L2TraceInd(bool par)
	{	
		if(IsStarted()) ThrowRuntimeException("Dss1 already started.");

		m_l2traceInd = par; 
	} 

	// end of property 
	// -------------------------------------------------------------------------------

	NObjDss1Interface *NObjDss1::FindBoard(int number) 
	{
		ListBoards::iterator i = std::find_if(m_boards.begin(), m_boards.end(), 
			boost::bind(&NObjDss1Interface::InterfaceNumber, _1) == number);
		
		return (i != m_boards.end()) ?  *i: 0;
	}

	// -------------------------------------------------------------------------------

	const NObjDss1Interface *NObjDss1::FindBoard(int number) const
	{
		ListBoards::const_iterator i = std::find_if(m_boards.begin(), m_boards.end(), 
			boost::bind(&NObjDss1Interface::InterfaceNumber, _1) == number);

		return (i != m_boards.end()) ?  *i: 0;
	}

	// -------------------------------------------------------------------------------

	NObjDss1Interface *NObjDss1::FindNextBoard(int number)
	{
		if(m_boards.size() == 0) return 0;

		if(number == -1) return m_boards.front();

		ListBoards::iterator i = std::find_if(m_boards.begin(), m_boards.end(), 
			boost::bind(&NObjDss1Interface::InterfaceNumber, _1) == number);

		ESS_ASSERT(i != m_boards.end());

		if(++i == m_boards.end()) i = m_boards.begin();
		return *i;

	}

	// -------------------------------------------------------------------------------	

	void NObjDss1::StartImpl()
	{
		ESS_ASSERT(IsStarted());
		
		if (IsLogActive(InfoTag)) Log(InfoTag) << "Started." << iLogW::EndRecord;
		
		m_stackIsdn.reset(new ISDN::IsdnStack(getDomain(), GetIsdnSetting(), 
			m_infra->IsdnInfra(), 
			m_driverL1->GetBinder(), 
			m_myLinkBinderStorage.getBinder<ISDN::ICallbackDss>(this)));

		m_l3Profile->PropertyWriteEventAbort("Owner already started.");

		m_boardStrategy.reset(new BoardOrderStrategy(*this));

		std::for_each(m_boards.begin(), m_boards.end(), 
			boost::bind(&NObjDss1Interface::Start, _1));
	}

	// -------------------------------------------------------------------------------

	void NObjDss1::StopImpl()
	{
		ESS_ASSERT(!IsStarted());

		std::for_each(m_boards.begin(), m_boards.end(), 
			boost::bind(&NObjDss1Interface::Stop, _1));

		if(m_calls.empty()) m_stackIsdn.reset();
		
		m_l3Profile->PropertyWriteEventClear();
		
		m_boardStrategy.reset();
		if (IsLogActive(InfoTag)) Log(InfoTag) << "Stopped." << iLogW::EndRecord;
	}

	// -------------------------------------------------------------------------------

	void NObjDss1::OnActivateDss1Stack()
	{
		if(!IsStarted()) return;

		if (IsLogActive(InfoTag)) Log(InfoTag) << "Activate stack" << iLogW::EndRecord;


		m_stackIsdn->GetDssIntf()->Activate();
	}

	// -------------------------------------------------------------------------------

	void NObjDss1::OnDeactivateDss1Stack()
	{
		if(!IsStarted()) return;

		if (IsLogActive(InfoTag)) Log(InfoTag) << "Deactivate stack" << iLogW::EndRecord;

		m_stackIsdn->GetDssIntf()->Deactivate();
	}

	// -------------------------------------------------------------------------------

	void NObjDss1::OnDeleteCall(IDss1Call*)	
	{
		if(IsStarted()) return;

		if(m_calls.empty()) m_stackIsdn.reset();
	}

	// -------------------------------------------------------------------------------

	ISDN::IsdnStackSettings NObjDss1::GetIsdnSetting() 
	{
		boost::shared_ptr<ISDN::L3Profile> l3Profile = 
			m_l3Profile->GenerateProfile(this, Name().Name().toStdString());
		if(m_boards.size() > 1) l3Profile->GetOptions().m_AssignBChannalsIntfId = true;
		
		ISDN::LapdTraceOption l2Trace;
		l2Trace.m_traceRaw = true;
		l2Trace.m_traceOn = L2TraceInd();
		boost::shared_ptr<ISDN::L2Profile> l2Profile = 
			(l3Profile->IsUserSide()) ? 
			ISDN::L2Profile::CreateAsTE(l2Trace, Name().Name().toStdString()) :
			ISDN::L2Profile::CreateAsNT(l2Trace, Name().Name().toStdString()); 

		l2Profile->setTei(0);
		l2Profile->setInitialState(false);		

		return ISDN::IsdnStackSettings(l2Profile, l3Profile);
	}

	// -------------------------------------------------------------------------------------

	void NObjDss1::SetState(StateType state, const std::string &comment)
	{
		m_state = state;

		RegistratorLog("State: (" + GetStateStr() + ") "+ comment.c_str());
	}

	// -------------------------------------------------------------------------------------

	void NObjDss1::RegistratorLog(const QString &str)
	{
		if (IsLogActive(InfoTag))
		{
			Log(InfoTag) << str.toStdString() << iLogW::EndRecord;
		}

		getDomain().SysReg().EventInfo(Name(), str);
	}

};


