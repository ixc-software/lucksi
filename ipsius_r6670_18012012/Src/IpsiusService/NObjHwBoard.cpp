#include "stdafx.h"

#include "NObjHwBoard.h"
#include "BfBootCli/TaskMngProfile.h"
#include "BfBootCli/TaskComposer.h"
#include "HwFirmware.h"

// #include "Utils/StateMachine.h"
#include "Utils/Fsm.h"
#include "HwBoardFsmStates.h"

#include "BfBootCore/BootControlProto.h"

namespace 
{
    const int CMinBooterVer = 1;
    const int CMaxBooterVer = 20;    
    BOOST_STATIC_ASSERT(CMinBooterVer <= CMinBooterVer);

    // Реальные периоды квантуются величеной NObjHwBoardList::CBoardProcessIntervalMs !
    const int CReloadTimeMsec = 3000; // ResetApp, App --> Booter  
    
    const bool CStBlockedRecovery = false;  //Allowed change StBlocked --> StOff by timeout
    const bool CStBlockedIsDeaf = true;     //Don`t msg process in StBlocked

    const bool CDebugTrace = false;

    QString ResolveHwType(const BfBootCore::BroadcastMsg &msg)
    {
        if (msg.HwType == 1) return "E1";
        if (msg.HwType == 2) return "AOZ";
        if (msg.HwType == 3) return "Pult";
        return QString("Type%1").arg(msg.HwType);
    }

	QString BoardName(const BfBootCore::BroadcastMsg &msg)
	{
		return QString("%1_%2").arg(ResolveHwType(msg)).arg(msg.HwNumber);
	}    	    

} // namespace 

namespace IpsiusService
{
    using Utils::TimerTicks;
    using Utils::HostInf;
    using boost::scoped_ptr;

    class HwBoardFsmEvents
    {
    protected:

        struct BaseEvent 
        { 
            virtual ~BaseEvent() {}
        };

        struct EvWithMsg : public BaseEvent
        {
            const BfBootCore::BroadcastMsg &Msg; 

            EvWithMsg(const BfBootCore::BroadcastMsg &msg) : Msg(msg) {}
        };

        #define DEF_EV(name)  struct name : public EvWithMsg { name(const BfBootCore::BroadcastMsg &msg) : EvWithMsg(msg) {} }  

        DEF_EV(EvDiscovered);  // Board discovered
        DEF_EV(EvMsgBoot0_Fwu0); // Board have unsupported booter version, fwu - need update
        DEF_EV(EvMsgApp_Fwu0); // message from App, fwu - need update
        DEF_EV(EvMsgApp_Fwu1);  // message from App, fwu Ok
        DEF_EV(EvMsgBoot1_Fwu1); // message from booter, fwu Ok
        DEF_EV(EvMsgBoot1_Fwu0); // message from booter, fwu - need update
        
        DEF_EV(EvMsgBusy); // плата уже управляется c PC
        DEF_EV(EvMsgFree); // плата была занята и освободилась        
        //... DEF_EV();

        #undef DEF_EV

        struct EvConflict : public BaseEvent 
        { 
            QString Descr;
            const BfBootCore::BroadcastMsg& Msg;
            EvConflict(QString descr, const BfBootCore::BroadcastMsg& msg): Descr(descr), Msg(msg) {} 
        };

        struct EvBroadcastTimeout : public BaseEvent {};
        struct EvUpdateComplete : public BaseEvent {};
        struct EvUpdateFail : public BaseEvent  { QString Descr; EvUpdateFail(QString descr) : Descr(descr){} };

        struct EvBooterTime : public BaseEvent {}; // предпологаемый момент готовности BootServer после зброса приложения
        //struct EvAppTime {}; // предпологаемый момент загрузки приложения после обновления прошивки
        // ...

    };    
    
    // ------------------------------------------------------------------------------------

    class NObjHwBoard::HwBoardFsm : 
        Utils::FsmSpecState,  // mix enums 
        HwBoardFsmEvents, // mix event definition        
        HwBoardFsmStates,  // mix states definition
        BfBootCli::ITaskManagerReport
    {
        typedef NObjHwBoard::HwBoardFsm T;
        typedef Utils::Fsm<BaseEvent, States> Fsm;

        BOOST_STATIC_ASSERT(BfBootCore::CBroadcastSendPeriod <= 2000);

        enum
        {
            CBroadcastTimeoutSec = 30,            
            CChangeIpLimitSec = 5, 
        };

    public:

        HwBoardFsm(IHwBoardOwner &owner, NObjHwBoard& wrap, iLogW::LogSession& log,
            const BfBootCore::BroadcastMsg& msg) : 
            m_fsm(StInitial, false, boost::bind(&T::OnNoEventHandle, this, _1),
                                    boost::bind(&T::OnStateChange, this, _1, _2) ),
            m_owner(owner),            
            m_wrap(wrap),
            m_isDirect(owner.IsDirect(msg)),
            m_isFiltered(owner.IsFiltered(msg)),            
            m_log(log),
            m_traceSrc(BoardName(msg)),
            m_disableTimeout(false)
        {   
            InitTable(m_fsm);
            m_fsm.DoEvent(EvDiscovered(msg));
        }

        ~HwBoardFsm()
        {
            m_fsm.SetState(StOff);
        }

        bool ProcessMsg( const BfBootCore::BroadcastMsg &msg )
        {   // check number
            if (msg.HwNumber != m_baseMsg.HwNumber || msg.HwType != m_baseMsg.HwType)
            {
                return false;
            }

            NewMsgReceived(msg);

            return true;
        }      
        
        void TimeProcess()
        {
            if (!CStBlockedRecovery && m_fsm.GetState() == StBlocked) return;
            if (m_fsm.GetState() == StOff) return;

            if (TimeOut(m_tBooterActivated)) m_fsm.DoEvent(EvBooterTime());                             

            // timeout check
            if (m_disableTimeout) return;
            QDateTime curr = QDateTime::currentDateTime();
            if (m_lastRecv.secsTo(curr) > CBroadcastTimeoutSec) m_fsm.DoEvent(EvBroadcastTimeout());            
        }

        bool CleanRequired() const
        {
            return (m_fsm.GetState() == StOff || m_fsm.GetState() == StBlocked);
        }        

        const BfBootCore::BroadcastMsg& Msg() const { return m_baseMsg; }
        bool IsDirect() const                       { return m_isDirect; }
        bool IsFiltered() const                     { return m_isFiltered; }
        QDateTime getFirstRecv() const              { return m_firstRecv; }
        QDateTime getLastRecv() const               { return m_lastRecv; }
        QDateTime getStateEnter() const             { return m_stateEnter; }

        QString MakeStateDesc() const
        {
            QString res = m_stateDesc;

            // state custom code
            if ( (m_fsm.GetState() == StService) && (m_updateTask != 0) )
            {
                QString info = m_updateTask->StateInfo();

                if (!info.isEmpty())
                {
                    res += "; "; 
                    res += info;
                }
            }

            return res;
        }

        QString StateStr()
        {
            return ResolveState(m_fsm.GetState());
        }

    // ITaskManagerReport impl
    private:
        void AllTaskFinished(const BfBootCli::TaskResult &result)
        {
            if (m_updateTask == 0) return;       

            // log
            if (m_owner.BoardTraceEnabled())
            {
                QString msg = QString("Update task completed; %1").arg(result.ToString());
                m_owner.BoardTrace(m_traceSrc, msg);
            }
            
            if (result.Ok)
                m_fsm.DoEvent(EvUpdateComplete());
            else
            {
                ESS_ASSERT(m_pFirmware != 0);                               
                QString descr = QString("Software update fail: %1; FW %2")
                    .arg(result.ToString()).arg(m_pFirmware->Info());   

                m_fsm.DoEvent(EvUpdateFail(descr));                                 
            }                        
            
            Utils::IVirtualInvoke *pI 
                = Utils::VirtualInvoke::Create(&HwBoardFsm::ResetUpdateTask, *this);
            m_wrap.getDomain().getMsgThread().AsyncVirtualInvoke(pI);            
        }

        void Info(QString eventInfo, int val)
        {
            if (!m_owner.BoardTraceEnabled() || !m_owner.TraceProgress()) return;
            if (val != -1 && val % 10 != 0) return;
            QString msg = (val == -1) ? eventInfo : QString("%1 %2").arg(eventInfo).arg(val);
            m_owner.BoardTrace(m_traceSrc, msg);              
        }

    // Override fsm methods:
    private:                                      
        
        void OnNoEventHandle(const BaseEvent &e) // OnNoEventHandle не дает изменить состояние
        {
            QString err = QString("Unexpected event: %1 in state %2")// \nMsgInfo:\n%3")
                .arg(typeid(e).name())
                .arg( ResolveState( m_fsm.GetState() ) );
            //.arg(Msg().Info().c_str());            

            if (m_owner.BoardTraceEnabled())
                m_owner.BoardTrace(m_traceSrc, err);                
            
            SetNotAvailableDescr(err);
            m_fsm.SetState(StBlocked);
        }

        void OnStateChange(States prevState, States newState)
        {
            if (newState == prevState) return;        

            if (m_owner.BoardTraceEnabled())
            {
                QString msg = QString("FSM %1 -> %2").arg( ResolveState(prevState) ).arg( ResolveState(newState) );

                m_owner.BoardTrace(m_traceSrc, msg);
            }

            if (prevState == StReady)
            {
                if (m_isFiltered) m_owner.BoardLeaveReadyState(m_wrap);
            }
            
            if (newState == StReady && m_isFiltered)
            {
                m_owner.BoardEnterReadyState(m_wrap);
            }                                    
            m_stateEnter = QDateTime::currentDateTime();                                  

            if (newState == StBlocked)  
            {
                QString dsc = QString("%1-->%2 Reason: ")
                    .arg(ResolveState(prevState)).arg(ResolveState(newState));
                m_stateDesc.prepend(dsc);
            }
            else
                m_stateDesc = ResolveState(newState);           

        }

    // event processors:
    private:

        void Nop(const BaseEvent &ev) {}
       
        void Discover(const EvDiscovered& ev)
        {
            // reset vars
            m_pFirmware.reset();
            m_updateTask.reset();
            m_wrap.InkDiscoverCounter();
            m_wrap.InkBroadcastCounter();            
            m_firstRecv = m_lastRecv = QDateTime::currentDateTime();            

            m_baseMsg = ev.Msg;            
        }

        void ResolveConflict(const EvConflict& ev)
        {
            QString errMsg = "Conflicted: " + ev.Descr;

            if (m_owner.BoardTraceEnabled())
            {
                m_owner.BoardTrace(m_traceSrc, errMsg);
            }
            
            m_wrap.InkConflictCounter();

            QDateTime curr = QDateTime::currentDateTime();            
            if (m_firstRecv.secsTo(curr) < CChangeIpLimitSec)
            {   
                SetNotAvailableDescr(errMsg);
                m_fsm.SetState(StBlocked);                                
            }
            else
            {                        
                m_fsm.DoEvent( EvDiscovered(ev.Msg) );
                NewMsgReceived(ev.Msg);
            }                        
        }
        
        void Wakeup(const EvWithMsg &ev) // обнаружение платы после StOff, StBusy
        {
            m_fsm.SetState(StInitial);
            ProcessMsg(ev.Msg);
        } 

        void GoBusy(const EvMsgBusy& ev)
        {
            m_stateDesc = QString("Busy by owner %1").arg(ev.Msg.OwnerAddress.c_str());
        }

        void BadBooterDescr(const EvMsgBoot0_Fwu0& ev)
        {                        
            QString desc = QString("Bad booter version %1, wait %2-%3 ")
                .arg(ev.Msg.BooterVersionNum).arg(CMinBooterVer).arg(CMaxBooterVer);

            SetNotAvailableDescr(desc);
        }

        void ResetBoard(const EvMsgApp_Fwu0& ev)
        {
            m_owner.ResetBoard(ev.Msg);

            if (!m_isDirect) return;
            
            // generate EvBooterTime by timer
            m_tBooterActivated.reset(new TimerTicks);
            m_tBooterActivated->Set(CReloadTimeMsec);

            m_disableTimeout = true;
        }

        void BeginFwUpdate(const EvMsgBoot1_Fwu0& ev)
        {
            StartUpdate(ev.Msg.CbpAddress);
        }

        void BeginFwUpdateTimer(const EvBooterTime&)
        {
            HostInf cbpAddress(m_baseMsg.SrcAddress);
            cbpAddress.Port(BfBootCore::CDefaultCbpPort);

            StartUpdate(cbpAddress);
        }        
        
        void UpdateFailedDescr(const EvUpdateFail& ev)
        {            
            SetNotAvailableDescr(ev.Descr);
        }       

        void EnableTimeoutIfDirect(const EvMsgApp_Fwu1&)
        {
            if (!m_isDirect) return; // nop
            m_disableTimeout = false;
        }        

    // own methods;
    private:
        void ResetUpdateTask()
        {
            m_updateTask.reset();
        }               

        void SetNotAvailableDescr(QString descr)
        {
            m_stateDesc = descr;
        }

        /*
        // for debug only
        template<class T>
        int m_fsm.DoEvent(const T& ev)
        {            
            if (CDebugTrace && m_owner.BoardTraceEnabled())
            {
                QString evName = QString("%1 in state %2")
                    .arg(typeid(T).name())
                    .arg( ResolveState(m_fsm.GetState()) );
                m_owner.BoardTrace(m_traceSrc, evName);
            }
            return Utils::StateMachine<HwBoardFsm>::m_fsm.DoEvent(ev);
        }
        */

        // event generator
        void NewMsgReceived(const BfBootCore::BroadcastMsg& msg)
        {               
            if (CStBlockedIsDeaf && m_fsm.GetState() == StBlocked) return;     
            if (m_fsm.GetState() == StInitial) m_fsm.DoEvent( EvDiscovered(msg) );                       
            
            QString diffDesc;
            if (m_fsm.GetState() != StOff && !UpdateBaseMsg(msg, diffDesc))                                             
            {
                m_fsm.DoEvent( EvConflict(diffDesc, msg) );
                return;
            }

            // stats update            
            m_wrap.InkBroadcastCounter();
            m_lastRecv = QDateTime::currentDateTime();
                
            if (m_fsm.GetState() == StBusy && msg.OwnerAddress.empty()) // плата освободилась            
            {
                m_fsm.DoEvent(EvMsgFree(msg) );                       
                return;
            }
            
            if (!msg.OwnerAddress.empty()) 
            {                                
                m_fsm.DoEvent( EvMsgBusy(msg) );            
                return;
            }                       

            // request FW
            if (m_isFiltered) m_pFirmware = m_owner.FirmwareRequest(msg);

            if (m_pFirmware == 0)  // where is no firmware for update
            {
                if (msg.IsBooter()) m_fsm.DoEvent(EvMsgBoot1_Fwu1(msg));
                else m_fsm.DoEvent(EvMsgApp_Fwu1(msg));
            }
            else  // update required
            {                                
                if (msg.BooterVersionNum < CMinBooterVer || msg.BooterVersionNum > CMaxBooterVer)                 
                {
                    m_fsm.DoEvent(EvMsgBoot0_Fwu0(msg));                    
                    return;
                }

                if (msg.IsBooter())                 
                    m_fsm.DoEvent(EvMsgBoot1_Fwu0(msg));
                else                
                    m_fsm.DoEvent(EvMsgApp_Fwu0(msg));                                           
            }                 
        }

        bool UpdateBaseMsg( const BfBootCore::BroadcastMsg &msg, QString &diffDesc )
        {            
            if (m_baseMsg.SrcAddress.Address() != msg.SrcAddress.Address()) 
            {
                diffDesc = QString("IP changed %1 -> %2")
                    .arg(m_baseMsg.SrcAddress.ToString().c_str())
                    .arg(msg.SrcAddress.ToString().c_str());

                return false;
            }

            if (msg.IsBooter())
            {
                m_baseMsg.SoftReleaseInfo = msg.SoftReleaseInfo;
                m_baseMsg.SoftRelNumber = msg.SoftRelNumber;
                m_baseMsg.BooterVersionInfo = msg.BooterVersionInfo;
                m_baseMsg.BooterVersionNum = msg.BooterVersionNum;
                m_baseMsg.BooterPort = msg.BooterPort;
                m_baseMsg.CbpAddress = msg.CbpAddress;
            }
            else
            {
                m_baseMsg.CmpPort = msg.CmpPort;
                m_baseMsg.BooterPort = msg.BooterPort;
            }

            m_baseMsg.OwnerAddress = msg.OwnerAddress;

            return true;
        }

        void StartUpdate(const HostInf& cbpAdress)
        {        
            ESS_ASSERT(m_pFirmware != 0);                        
            
            // begin update
            BfBootCli::TaskMngProfile profile(m_wrap, 
                *this, 
                m_owner.TaskMngTraceEnabled(), 
                m_log.LogCreator());

            if (m_owner.BoardTraceEnabled())
            {
                QString msg = QString("Begin update, fwu #%1 ...")
                    .arg( m_pFirmware->Release() );               
                m_owner.BoardTrace(m_traceSrc, msg);
            }

            BfBootCli::SbpTcpCreator tcpCreator(cbpAdress, BfBootCore::CSplitSizeForSendToBooterByTcp);
            m_updateTask = 
                BfBootCli::TaskComposer::UpdateFirmware(profile, 
                tcpCreator,
                "", 
                *m_pFirmware);            
        }

        static bool TimeOut(scoped_ptr<TimerTicks>& t)
        {
            if (t == 0 || !t->TimeOut()) return false;
            t.reset();            
            return true;
        }


    // fsm transition table:
    private:
        
        void InitTable(Fsm &fsm)
        {
            fsm.EnableRecursiveMode();

            #define ADD UTILS_FSM_ADD

            ADD(_AnyState_,  EvDiscovered, Discover, StDiscovered);
            ADD(_AnyState_,  EvConflict,   ResolveConflict, _DynamicState_); // discover or NotAvailable

            // EvEnyMsg --> EnterDiscovered
            ADD(StOff,  EvMsgApp_Fwu0,      Wakeup, StDiscovered);
            ADD(StOff,  EvMsgApp_Fwu1,      Wakeup, StDiscovered);
            ADD(StOff,  EvMsgBoot1_Fwu1,    Wakeup, StDiscovered);
            ADD(StOff,  EvMsgBoot1_Fwu0,    Wakeup, StDiscovered);            

            ADD(StBusy,        EvMsgFree,   Wakeup, StDiscovered);
            ADD(StDiscovered,  EvMsgBusy,   GoBusy, StBusy);

            ADD(StDiscovered,  EvMsgBoot1_Fwu1, Nop, StWaitBooterExit);
            ADD(StDiscovered,  EvMsgApp_Fwu1,   Nop, StReady);
            
            ADD(StDiscovered,       EvMsgBoot1_Fwu0, BeginFwUpdate, StService);
            ADD(StWaitBooterEnter,  EvMsgBoot1_Fwu0, BeginFwUpdate, StService);     
            ADD(StDiscovered,       EvMsgApp_Fwu0,   ResetBoard, StWaitBooterEnter);
            ADD(StDiscovered,       EvMsgBoot0_Fwu0, BadBooterDescr, StBlocked);

            ADD(StService,  EvUpdateComplete, Nop,               StWaitBooterExit);
            ADD(StService,  EvUpdateFail,     UpdateFailedDescr, StBlocked);

            ADD(StWaitBooterExit, EvMsgApp_Fwu1, EnableTimeoutIfDirect, StReady);

            // ignore duplicate message:
            ADD(StWaitBooterExit,   EvMsgBoot1_Fwu1, Nop, _SameState_);
            ADD(StReady,            EvMsgApp_Fwu1,   Nop, _SameState_);
            ADD(StBusy,             EvMsgBusy,       Nop, _SameState_);
            ADD(StService,          EvMsgBusy,       Nop, _SameState_);
            ADD(StService,          EvMsgBoot1_Fwu0, Nop, _SameState_);
            ADD(StReady,            EvMsgBusy,       Nop, _SameState_);
            ADD(StWaitBooterEnter,  EvMsgApp_Fwu0,   Nop, _SameState_);

            ADD(StReady, EvMsgBoot1_Fwu1, Nop, StDiscovered);

            ADD(StWaitBooterEnter, EvBooterTime, BeginFwUpdateTimer, StService);            
            ADD(_AnyState_, EvBroadcastTimeout,  Nop,           StOff); 

            #undef ADD
        }

    // fields:
    private:

        Fsm m_fsm;

        IHwBoardOwner &m_owner;
        NObjHwBoard& m_wrap;        
        BfBootCore::BroadcastMsg m_baseMsg;
        const bool m_isDirect;
        const bool m_isFiltered;

        boost::shared_ptr<HwFirmware> m_pFirmware;
        boost::shared_ptr<BfBootCli::ITaskManager> m_updateTask;        

        QDateTime m_firstRecv, m_lastRecv, m_stateEnter;        
        
        iLogW::LogSession& m_log;        
        QString m_traceSrc;
        QString m_stateDesc;    
        
        scoped_ptr<Utils::TimerTicks> m_tBooterActivated; // ожидание готовности загрузчика после 
                                                          // сброса приложения (m_isDirect)
        bool m_disableTimeout;        
    };

} // namespace IpsiusService



// ------------------------------------------------------------------------------------

// NObjHwBoard methods:
namespace IpsiusService
{
    NObjHwBoard::NObjHwBoard( Domain::NamedObject *pParent, IHwBoardOwner &owner, const BfBootCore::BroadcastMsg &msg)
        : NamedObject( &pParent->getDomain(), BoardName(msg), pParent ),
        m_owner(owner),               		
		m_log(getDomain().LogCreator()->CreateSession(Name().GetShortName().toStdString(), true)),                             
        m_discovered(0),
        m_conflictsCount(0),
        m_broadcastCounter(0)
    {
        m_shortPropList.append("HwID");
        m_shortPropList.append("HwNumber");
        m_shortPropList.append("IP");
        m_shortPropList.append("StateDesc");

        m_longPropList.append(m_shortPropList);
        m_longPropList.append("MAC");
        m_longPropList.append("StateEnter");
        m_longPropList.append("Owner");
        m_longPropList.append("BooterPort");
        m_longPropList.append("CmpPort");
        m_longPropList.append("BootRelease");
        m_longPropList.append("BootReleaseInfo");
        m_longPropList.append("SoftRelease");
        m_longPropList.append("SoftReleaseInfo");
        m_longPropList.append("FirstRecv");
        m_longPropList.append("LastRecv");
        m_longPropList.append("DiscoveredCount");
        m_longPropList.append("BroadcastCounter");       

        m_fsm.reset( new HwBoardFsm(owner, *this, *m_log, msg) );         
    }

    // ------------------------------------------------------------------------------------

    NObjHwBoard::~NObjHwBoard()
    {}

    // ------------------------------------------------------------------------------------

    Q_INVOKABLE void NObjHwBoard::ListInfo( DRI::ICmdOutput *pCmd, bool briefly ) const
    {
        const QStringList& list = briefly ? m_shortPropList : m_longPropList;
        for (int i = 0; i < list.size(); ++i)
        {            
            pCmd->Add( list.at(i), false );
            pCmd->Add( " = ", false );              
            pCmd->Add(Property(list.at(i), false), false);
            pCmd->Add("; ", i == (list.size() - 1) );
        }
    }            
    
    // ------------------------------------------------------------------------------------
    // Redirect to implementation 

    bool NObjHwBoard::ProcessMsg( const BfBootCore::BroadcastMsg &msg )
    {
        return m_fsm->ProcessMsg(msg);        
    }

    void NObjHwBoard::TimeProcess()
    {
        m_fsm->TimeProcess();
    }

    bool NObjHwBoard::CleanRequired() const
    {
        return m_fsm->CleanRequired();
    }

    const BfBootCore::BroadcastMsg& NObjHwBoard::Msg() const
    {
        return m_fsm->Msg();
    }   

    int NObjHwBoard::HwNumber() const
    {
        ESS_ASSERT(m_fsm);

        return m_fsm->Msg().HwNumber;
    }

    bool NObjHwBoard::IsFiltered() const
    {
        return m_fsm->IsFiltered();
    }

    bool NObjHwBoard::IsDirect() const
    {
        return m_fsm->IsDirect();
    }

    int NObjHwBoard::getHwNumber() const
    {
        return m_fsm->Msg().HwNumber;
    }

    int NObjHwBoard::getHwID() const
    {
        return m_fsm->Msg().HwType;
    }

    QString NObjHwBoard::getIP() const
    {
        return m_fsm->Msg().SrcAddress.Address().c_str();
    }

    QString NObjHwBoard::getMAC() const
    {
        return m_fsm->Msg().Mac.c_str();
    }

    int NObjHwBoard::getBooterPort() const
    {
        return m_fsm->Msg().BooterPort;
    }

    int NObjHwBoard::getCmpPort() const
    {
        return m_fsm->Msg().CmpPort;
    }

    int NObjHwBoard::getBootRelease() const
    {
        return m_fsm->Msg().BooterVersionNum;
    }

    QString NObjHwBoard::getBootReleaseInfo() const
    {
        return m_fsm->Msg().BooterVersionInfo.c_str();
    }

    int NObjHwBoard::getSoftRelease() const
    {
        return m_fsm->Msg().SoftRelNumber;
    }
    
    QString NObjHwBoard::getSoftReleaseInfo() const
    {
        return m_fsm->Msg().SoftReleaseInfo.ToString().c_str();
    }

    QString NObjHwBoard::getStateDesc() const
    {
        return m_fsm->MakeStateDesc();
    }

    QString NObjHwBoard::getOwner() const
    {
        return m_owner.GetBoardOwner(*this);
    }

    QDateTime NObjHwBoard::getFirstRecv() const
    {
        return m_fsm->getFirstRecv();
    }

    QDateTime NObjHwBoard::getLastRecv() const
    {
        return m_fsm->getLastRecv();
    }

    QDateTime NObjHwBoard::getStateEnter() const
    {
        return m_fsm->getStateEnter();
    }

    QString NObjHwBoard::getState() const
    {
        return m_fsm->StateStr();
    }

    void NObjHwBoard::InkDiscoverCounter()
    {
        m_broadcastCounter = 0;
        ++m_discovered;
    }

}  // namespace IpsiusService

