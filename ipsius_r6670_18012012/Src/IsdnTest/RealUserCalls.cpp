#include "stdafx.h"

#include "RealUserCalls.h"
#include "UserCallInternalFsmBase.h"
#include "ISDN/DssUpIntf.h"
#include "NObjDssTestSettings.h"
#include "NObjSimulLiveSettings.h"

namespace IsdnTest
{
    using ISDN::DssCallParams;
    using ISDN::DssOutCallParams;
    using ISDN::IeConstants;

    #define ADD UTILS_FSM_ADD

    // ------------------------------------------------------------

    class TypicalOutCall::InternalFsm : public InternalFsmBaseForOutCall
    {        
        typedef InternalFsm T;

        TypicalOutCall& m_owner;

        shared_ptr<const DssCause> m_waitedCause;
        const bool m_useAlerting;

        void OnEvRun(const EvRun&)
        {
            ISubsToUserCall& subs = m_owner.getSubs();
            DssCallParams *pParam = DssOutCallParams::Create( subs.getInfra(), subs.getNumReceiver() );        

            ISDN::ILayerDss::CreateOutCallParametrs params(m_owner.getMyBinder(), shared_ptr<DssCallParams>(pParam));
            m_owner.getStack().GetDssIntf()->CreateOutCall(params);
        }

        void OnEvDssCallCreated(const EvDssCallCreated &e)
        {
            const ISDN::IDssCallEvents::DssCallCreatedParametrs &ev = e.m_params;

            //TUT_ASSERT(0 && "Test TUTexcepton");
            TUT_ASSERT (ev.m_setBCannels.Count() == 1);        
            //TODO сравнить номера назначенных каналов
            ev.m_linkBinder->Connect( m_owner.getLinkToDssCall() ); //set link
            //m_owner.m_link = ev.m_link;

            if (m_useAlerting) FSM().SetState(st_waitAlerting);
            else               FSM().SetState(st_waitConnected);
        }

        void OnEvConnected(const EvConnected&)
        {
            //TUT_ASSERT(0 && "Test TUTexcepton");
            ISubsToUserCall& subs = m_owner.getSubs();
            DoDisc();
        }

        void OnEvConnectConf(const EvConnectCnf&)
        {
            DoDisc();
        }

        void DoDisc()
        {
            Link& link = m_owner.getLinkToDssCall();

            link->Disconnent(
                DssCause::Create(m_owner.getStack().GetL3Ptr(), IeConstants::NormCallClearing) 
                );

            link.Disconnect();
        }

        void OnEvDiscByDrop(const EvDisconnent &ev)
        {
            if (*ev.m_cause == *m_waitedCause ) 
            {
                FSM().SetState(st_complete);
            }
        }

        void Drop(shared_ptr<const DssCause> cause, IncommingDssUserCall& opposide) //override
        {            
            if ( !m_owner.getLinkToDssCall().Connected() ) 
            {                
                if (FSM().GetState() != st_complete)//ещё дисконект не послан
                {
                    shared_ptr<const DssCause> cause (
                        DssCause::Create(m_owner.getStack().GetL3Ptr(), IeConstants::CallRejected) 
                        );
                    opposide.OpposideDropped(cause); // ожидать Reject
                }
                //m_owner.getStack().GetLocation();
                m_owner.DeleteThisCall();             
                return; 
            }
            ISubsToUserCall& subs = m_owner.getSubs();
            m_owner.getLinkToDssCall()->Disconnent(cause );
            m_owner.getLinkToDssCall().Disconnect(); // блокировка линка
            opposide.OpposideDropped(cause);
            FSM().SetState(st_complete);
        }

        void OpposideDropped(shared_ptr<const DssCause> cause) //override
        {
            ESS_ASSERT(!m_waitedCause.get());            
            if (FSM().GetState() == st_complete) return;

            m_waitedCause = cause;
            FSM().SetState(st_waitDiscWithCause);            
        }

        void InitTable(Fsm &fsm)
        {
            ADD(st_null,                    EvRun,              OnEvRun,                st_waitDssCallCreated);
            ADD(st_waitDssCallCreated,      EvDssCallCreated,   OnEvDssCallCreated,     _DynamicState_);
            ADD(st_waitAlerting,            EvAlerting,         Nop,                    st_waitConnected);
            ADD(st_waitConnected,           EvConnected,        OnEvConnected,          st_complete);
            ADD(st_complete,                EvLinkDisc,         Nop,                    st_complete);
            ADD(st_waitDiscWithCause,       EvDisconnent,       OnEvDiscByDrop,         _DynamicState_);
            ADD(st_waitDiscWithCause,       EvDssCallCreated,   Nop,                    st_waitDiscWithCause);
            ADD(st_waitDiscWithCause,       EvAlerting,         Nop,                    st_waitDiscWithCause);
            ADD(st_waitDiscWithCause,       EvConnected,        Nop,                    st_waitDiscWithCause);
        }

    public:

        InternalFsm(TypicalOutCall& owner, bool useAlerting)
            : InternalFsmBaseForOutCall(owner.m_logSession),
            m_owner(owner),
            m_useAlerting(useAlerting)
        {
            InitTable( FSM() );
        }
    };

    // ------------------------------------------------------------

    TypicalOutCall::TypicalOutCall(ISubsToUserCall& subs, bool useAlerting)
        : OutgoingDssUserCall(subs),
        m_logSession(subs.getLogger(), this),
        m_fsm( new InternalFsm(*this, useAlerting) )        
    {
        SetFsm( m_fsm.get() );        
    }

    // ------------------------------------------------------------

    class TypicalInCall::InternalFsm : public InternalFsmBaseForInCall
    {
        typedef InternalFsm T;

        bool m_afterDrop;
        const bool m_useAlerting;

        TypicalInCall& m_owner;  
        shared_ptr<const DssCause> m_waitedCause;

        void OnEvIncoming(const EvIncomingCall& ev)
        {
            ISubsToUserCall& subs = m_owner.getSubs();        
            ESS_ASSERT(ev.m_pParams->GetCalled().Digits() == subs.getNumReceiver().Digits() );
            TUT_ASSERT(ev.m_pParams->GetReqBchansCount() == 1);
            Link& link = m_owner.getLinkToDssCall();

            ev.m_dssCallLink->Connect(link); // link connect        

            link->SetLink( m_owner.getMyBinder() );

            link->AddressComplete();                // разрешаем завершить Setup
            if (m_useAlerting) link->Alerting();    // КПВ
            link->ConnectRsp();                     // Принимаем вызов        
        }

        void OnEvDisconnect(const EvDisconnent &ev)
        {
            TUT_ASSERT(ev.m_cause.get() && "EmptyCause!");
            m_owner.getLinkToDssCall().Disconnect(); // lock link
        }

        void OnEvDiscByDrop(const EvDisconnent &ev)
        {
            TUT_ASSERT(ev.m_cause.get() && "EmptyCause!");
            if (*ev.m_cause == *m_waitedCause )
                FSM().SetState(st_complete);
        }

        void OnInAfterDrop(const EvIncomingCall& ev)
        {
            if (!m_afterDrop) TUT_ASSERT("Unexpected Event");
        }

        void Drop(shared_ptr<const DssCause> cause, OutgoingDssUserCall& opposide) //override
        {    
            //m_afterDrop = true;
            St state = FSM().GetState();
            if (state == st_null || state == st_waitIncommingCall) return;

            if ( !m_owner.getLinkToDssCall().Connected() )
            {
                shared_ptr<const DssCause> cause (
                    DssCause::Create(m_owner.getStack().GetL3Ptr(), IeConstants::CallRejected) 
                    );
                opposide.OpposideDropped(cause); // ожидать Reject

                m_owner.DeleteThisCall();             
                return; 
            }

            ISubsToUserCall& subs = m_owner.getSubs();
            m_owner.getLinkToDssCall()->Disconnent(cause );
            m_owner.getLinkToDssCall().Disconnect(); // блокировка линка
            opposide.OpposideDropped(cause);
            FSM().SetState(st_complete);
        }

        void OpposideDropped(shared_ptr<const DssCause> cause) //override
        {
            m_afterDrop = true;

            St state = FSM().GetState();

            if ( state == st_complete)
                return;
            if ( state == st_waitIncommingCall )
            {
                FSM().SetState(st_complete);
                return;
            }
            ESS_ASSERT(!m_waitedCause.get());            
            
            m_waitedCause = cause;            
            FSM().SetState(st_waitDiscWithCause);                      
        }

        void InitTable(Fsm &fsm)
        {
            ADD(st_null,                EvRun,          Nop,            st_waitIncommingCall);
            ADD(st_waitIncommingCall,   EvIncomingCall, OnEvIncoming,   st_waitConnConfirm);    // st_waitDisconnected
            ADD(st_waitConnConfirm,     EvConnectCnf,   Nop,            st_waitDisconnected);
            ADD(st_waitDisconnected,    EvDisconnent,   OnEvDisconnect, st_waitLinkDisc);
            ADD(st_waitLinkDisc,        EvLinkDisc,     Nop,            st_complete);
            ADD(st_complete,            EvConnectCnf,   Nop,            _SameState_);
            ADD(st_complete,            EvDisconnent,   Nop,            _SameState_);
            ADD(st_complete,            EvLinkDisc,     Nop,            _SameState_);
            ADD(st_complete,            EvIncomingCall, OnInAfterDrop,  _SameState_);
            ADD(st_waitDiscWithCause,   EvDisconnent,   OnEvDiscByDrop, _DynamicState_);
            ADD(st_waitDiscWithCause,   EvConnectCnf,   Nop,            _SameState_);
        }

    public:

        InternalFsm(TypicalInCall& owner, bool useAlerting) : 
            InternalFsmBaseForInCall(owner.m_logSession),
            m_owner(owner),
            m_afterDrop(false),
            m_useAlerting(useAlerting)
        {
            InitTable( FSM() );
        }
    };

    // ------------------------------------------------------------

    TypicalInCall::TypicalInCall(ISubsToUserCall& subs, bool useAlerting)
        : IncommingDssUserCall(subs),
        m_logSession(subs.getLogger(), this),
        m_fsm( new InternalFsm(*this, useAlerting) )        
    {
        SetFsm( m_fsm.get() );
    }

    // -------------------------------------------------------------------

    class OutCreateConnect::InternalFsm : public InternalFsmBaseForOutCall
    {
        typedef InternalFsm T;

        OutCreateConnect &m_owner;

        void OnEvRun(const EvRun&)
        {
            ISubsToUserCall& subs = m_owner.getSubs();
            DssCallParams *pParam = DssOutCallParams::Create( subs.getInfra(), subs.getNumReceiver() );
            ISDN::ILayerDss::CreateOutCallParametrs params(m_owner.getMyBinder(), shared_ptr<DssCallParams>(pParam));
            m_owner.getStack().GetDssIntf()->CreateOutCall(params);
        }

        void OnEvDssCallCreated(const EvDssCallCreated &ev)
        {
            TUT_ASSERT (ev.m_params.m_setBCannels.Count() == 1);                
            //ev.m_linkBinder->Connect(m_owner.getLinkToDssCall()); //set link        
        }

        void InitTable(Fsm &fsm)
        {
            ADD(st_null,                EvRun,              OnEvRun,            st_waitDssCallCreated);
            ADD(st_waitDssCallCreated,  EvDssCallCreated,   OnEvDssCallCreated, st_waitAlerting);
            ADD(st_waitAlerting,        EvAlerting,         Nop,                st_waitConnected);
            ADD(st_waitConnected,       EvConnected,        Nop,                st_complete);
            ADD(st_complete,            EvLinkDisc,         Nop,                st_complete);
            ADD(st_complete,            EvDisconnent,       Nop,                st_complete);
        }
        
    public:

        InternalFsm(OutCreateConnect& owner) : 
            InternalFsmBaseForOutCall(owner.m_logSession),
            m_owner(owner)
        {
            InitTable( FSM() );
        }
    };

    // ---------------------------------------------------------------------

    // Owner constructor
    OutCreateConnect::OutCreateConnect(ISubsToUserCall& subs)
        : OutgoingDssUserCall(subs),
        m_logSession(subs.getLogger(), this),
        m_fsm( new InternalFsm(*this) )
    {
        SetFsm( m_fsm.get() );        
    }

    // ---------------------------------------------------------------------

    class InCreateConnect::InternalFsm : public InternalFsmBaseForInCall
    {
        typedef InternalFsm T; 

        InCreateConnect& m_owner;

        void OnEvIncoming(const EvIncomingCall& ev)
        {
            ISubsToUserCall& subs = m_owner.getSubs();                
            ESS_ASSERT(ev.m_pParams->GetCalled().Digits() == subs.getNumReceiver().Digits());
            TUT_ASSERT(ev.m_pParams->GetReqBchansCount() == 1);
            Link& link = m_owner.getLinkToDssCall();

            ev.m_dssCallLink->Connect(link); // link connect        

            link->SetLink( m_owner.getMyBinder() );

            link->AddressComplete(); // разрешаем завершить Setup
            link->Alerting();//КПВ
            link->ConnectRsp();//Принимаем вызов        
        }

        void OnEvConnectCnf(const EvConnectCnf& ev)
        {
            m_owner.getLinkToDssCall().Disconnect();
        }

        void InitTable(Fsm &fsm)
        {
            ADD(st_null,                EvRun,          Nop,            st_waitIncommingCall);
            ADD(st_waitIncommingCall,   EvIncomingCall, OnEvIncoming,   st_waitConnConfirm);    // st_waitDisconnected
            ADD(st_waitConnConfirm,     EvConnectCnf,   OnEvConnectCnf, st_waitLinkDisc);
            ADD(st_waitLinkDisc,        EvLinkDisc,     Nop,            st_complete);
            ADD(st_complete,            EvDisconnent,   Nop,            st_complete);
        }
        
    public:

        InternalFsm(InCreateConnect& owner) : 
            InternalFsmBaseForInCall(owner.m_logSession),
            m_owner(owner)
        {
            InitTable( FSM() );
        }
    };

    // ---------------------------------------------------------------------

    // Owner constructor
    InCreateConnect::InCreateConnect(ISubsToUserCall& subs)
        : IncommingDssUserCall(subs),
        m_logSession(subs.getLogger(), this),
        m_fsm( new InternalFsm( *this ) )
    {
        SetFsm( m_fsm.get() );
    }

    // ---------------------------------------------------------------------

    class OutCheckBusyMsg::InternalFsm : public InternalFsmBaseForOutCall
    {
        typedef InternalFsm T;

        OutCheckBusyMsg& m_owner;

        void OnEvRun(const EvRun&)
        {        
            ISubsToUserCall& subs = m_owner.getSubs();
            DssCallParams *pParam = DssOutCallParams::Create( subs.getInfra(), subs.getNumReceiver() );
            ISDN::ILayerDss::CreateOutCallParametrs params(m_owner.getMyBinder(), shared_ptr<DssCallParams>(pParam));
            m_owner.getStack().GetDssIntf()->CreateOutCall(params);
        }

        void OnEvDisconnect(const EvDisconnent& ev)
        {
            if ( (ev.m_cause)->GetCauseNum() == IeConstants::NoCircChanAvailable )
                FSM().SetState(st_complete);
        }

        void InitTable(Fsm &fsm)
        {
            ADD(st_null,                EvRun,          OnEvRun,        st_waitDisconnected);
            ADD(st_waitDisconnected,    EvDisconnent,   OnEvDisconnect, _DynamicState_);
        }
        
    public:

        InternalFsm(OutCheckBusyMsg& owner) : 
            InternalFsmBaseForOutCall(owner.m_logSession),
            m_owner(owner)
        {
            InitTable( FSM() );
        }
    };

    // --------------------------------------------------------------

    // Owner constructor
    OutCheckBusyMsg::OutCheckBusyMsg(ISubsToUserCall& subs)
        : OutgoingDssUserCall(subs),
        m_logSession(subs.getLogger(), this),
        m_fsm( new InternalFsm(*this ) )
    {
        SetFsm( m_fsm.get() );        
    }

    // --------------------------------------------------------------

    class OutDroppedBeforeConnect::InternalFsm : public InternalFsmBaseForOutCall
    {
        enum {CTimerInterval = 10};
        St m_dropedState;        

        typedef InternalFsm T;

        OutDroppedBeforeConnect& m_owner;

        void OnAlerting( const EvAlerting& )
        {
            if (FSM().GetState() == m_dropedState)
                CloseCall();
            else
                FSM().SetState(st_waitConnected);
        }

        void OnEvRun(const EvRun& ev)
        {
            ISubsToUserCall& subs = m_owner.getSubs();
            DssCallParams *pParam = DssOutCallParams::Create( subs.getInfra(), subs.getNumReceiver() );
            ISDN::ILayerDss::CreateOutCallParametrs params(m_owner.getMyBinder(), shared_ptr<DssCallParams>(pParam));
            m_owner.getStack().GetDssIntf()->CreateOutCall(params);
        }

        void CloseCall()
        {
            Writer( *getLog() ).Write() << "\nCloseCall in state " << StateToString( FSM().GetState() );
            ISubsToUserCall& subs = m_owner.getSubs();
            Link& link = m_owner.getLinkToDssCall();
            if ( link.Connected() )
            {
                shared_ptr<const DssCause> cause (
                    DssCause::Create(m_owner.getStack().GetL3Ptr(), IeConstants::CallRejected) 
                    );//TODO CallRejected?

                m_owner.getLinkToDssCall()->Disconnent( cause );
                m_owner.getLinkToDssCall().Disconnect(); // блокировка линка
            }
            m_owner.AsyncDeleteThisCall();//? synhro!

            FSM().SetState(st_complete);
        }

        void OnCallCreated(const EvDssCallCreated& ev)
        {            
            ev.m_params.m_linkBinder->Connect( m_owner.getLinkToDssCall() );

            if (FSM().GetState() == m_dropedState)
                CloseCall();
            else
                FSM().SetState(st_waitAlerting);
        }

        void OnDisconnect(const EvDisconnent& ev)
        {
            // nothing 
        }

        void InitTable(Fsm &fsm)
        {
            ADD(st_null,                EvRun,              OnEvRun,        st_waitDssCallCreated);
            ADD(st_waitDssCallCreated,  EvDssCallCreated,   OnCallCreated, _DynamicState_);         // st_waitAlerting
            ADD(st_waitAlerting,        EvAlerting,         OnAlerting,     _DynamicState_);        // st_waitConnected
            ADD(st_waitDssCallCreated,  EvDisconnent,       OnDisconnect,   st_complete);           // no resource
            ADD(st_complete,            EvConnected,        Nop,            _SameState_);
            ADD(st_complete,            EvLinkDisc,         Nop,            _SameState_);
            ADD(st_complete,            EvAlerting,         Nop,            _SameState_);
        }

    public:

        InternalFsm(OutDroppedBeforeConnect& owner) :
            InternalFsmBaseForOutCall(owner.m_logSession),
            m_owner(owner)            
        {
            InitTable( FSM() );

            Utils::Random& rndObj = owner.getSubs().GetIScenario().GetRandom();

            // случайный выбор состояния в котором произойдет завершение
            int rndByte = rndObj.NextByte();

            if (rndByte > 255/2)
                m_dropedState = st_waitDssCallCreated;
            else                                            // if(rndByte > 255/3)
                m_dropedState = st_waitAlerting;
        }

        void OnDropTimer()
        {
            if (FSM().GetState() == m_dropedState)
            {
                m_owner.m_tDropTimer.Stop();
                TUT_ASSERT(0 && "No EvDrop() in table!"); // ProcessEvent( EvDrop() );
            }
        }
    };

    // -----------------------------------------------------------------

    OutDroppedBeforeConnect::OutDroppedBeforeConnect(ISubsToUserCall& subs)
        :  OutgoingDssUserCall(subs),
        m_logSession(subs.getLogger(), this),
        m_fsm( new InternalFsm(*this) ),
        m_tDropTimer(this, &OutDroppedBeforeConnect::OnDropTimer)
    {
        SetFsm( m_fsm.get() );        
    }

    void OutDroppedBeforeConnect::OnDropTimer(iCore::MsgTimer*)
    {
        m_fsm->OnDropTimer();
    }

    // -----------------------------------------------------------------

    // только принимает входящий вызов если он будет
    class InDroppedBeforeConnect::InternalFsm : public InternalFsmBaseForInCall
    {        
        typedef InternalFsm T;

        InDroppedBeforeConnect& m_owner;

        void OnEvIncoming(const EvIncomingCall& ev)
        {
            ISubsToUserCall& subs = m_owner.getSubs();            
            ESS_ASSERT(ev.m_pParams->GetCalled().Digits() == subs.getNumReceiver().Digits());
            TUT_ASSERT(ev.m_pParams->GetReqBchansCount() == 1);
            Link& link = m_owner.getLinkToDssCall();

            ev.m_dssCallLink->Connect(link); // link connect        
        
            //TODO ---> OnLinkConnected
            link->SetLink( m_owner.getMyBinder() );

            link->AddressComplete(); // разрешаем завершить Setup
            link->Alerting();//КПВ TODO by timer
            link->ConnectRsp();//Принимаем вызов TODO by timer
        }

        void OnEvConnectCnf(const EvConnectCnf& )
        {
            // nothing 
        }

        void InitTable(Fsm &fsm)
        {
            ADD(st_null,                EvRun,          Nop,            st_waitIncommingCall);
            ADD(st_waitIncommingCall,   EvIncomingCall, OnEvIncoming,   st_waitConnConfirm);
            ADD(st_waitConnConfirm,     EvConnectCnf,   OnEvConnectCnf, st_waitDisconnected);
            ADD(_AnyState_,             EvDisconnent,   Nop,            st_complete);
            ADD(_AnyState_,             EvLinkDisc,     Nop,            st_complete);
        }

    public:

        InternalFsm(InDroppedBeforeConnect& owner) : 
            InternalFsmBaseForInCall(owner.m_logSession),
            m_owner(owner)
        {
            InitTable( FSM() );
        }
    };

    // ------------------------------------------------------------

    InDroppedBeforeConnect::InDroppedBeforeConnect(ISubsToUserCall& subs)
        : IncommingDssUserCall(subs),
        m_logSession(subs.getLogger(), this),
        m_fsm( new InternalFsm(*this) )
    {
        SetFsm( m_fsm.get() );        
    }

    // ------------------------------------------------------------

    class OutTalkByTime::InternalFsm : public InternalFsmBaseForOutCall
    {
        typedef InternalFsm T;

        OutTalkByTime& m_owner;

        void OnEvRun(const EvRun&)
        {        
            ISubsToUserCall& subs = m_owner.getSubs();
            DssCallParams *pParam = DssOutCallParams::Create( subs.getInfra(), subs.getNumReceiver() );        

            ISDN::ILayerDss::CreateOutCallParametrs params(m_owner.getMyBinder(), shared_ptr<DssCallParams>(pParam));
            m_owner.getStack().GetDssIntf()->CreateOutCall(params);
        }

        void OnEvDssCallCreated(const EvDssCallCreated& e)
        {
            const ISDN::IDssCallEvents::DssCallCreatedParametrs &ev = e.m_params;

            //TUT_ASSERT(0 && "Test TUTexcepton");
            TUT_ASSERT (ev.m_setBCannels.Count() == 1);        
            
            ev.m_linkBinder->Connect(m_owner.getLinkToDssCall()); //set link            
        }

        void OnNoChannel(const EvDisconnent& ev)
        {
            Writer( *getLog() ).Write() << "Event disc " << ev.m_cause->ToString();
            //if (ev.m_cause->GetCauseNum() == 16)
            TUT_ASSERT (
                ev.m_cause->GetCauseNum() == ISDN::IeConstants::NoCircChanAvailable ||
                ev.m_cause->GetCauseNum() == ISDN::IeConstants::NoRequestedCircChanAvailable);
            m_owner.m_statistic.IncrRefused();
        }

        void OnEvConnected(const EvConnected &ev)
        {
            //m_owner.getLinkToDssCall()->ConnectRsp(); 

            if ( m_owner.TryStartTimer() ) // если я рву связь
            {
                FSM().SetState(st_waitSelfTimer);
                m_owner.DrawStat(); // соединение установленно, длительность разговора известна
            }
            else
                FSM().SetState(st_waitDisconnected);
        }

        void OnReleaseTalk(const EvReleaseTalk&)
        {
            Writer( *getLog() ).Write() << "\nOnReleaseTalk ";
            ISubsToUserCall& subs = m_owner.getSubs();
            Link& link = m_owner.getLinkToDssCall();
            
            shared_ptr<const DssCause> cause (
                DssCause::Create(m_owner.getStack().GetL3Ptr(), IeConstants::NormCallClearing) );//TODO causeNum ???
            m_owner.getLinkToDssCall()->Disconnent( cause );
            m_owner.getLinkToDssCall().Disconnect(); // блокировка линка
        }

        void InitTable(Fsm &fsm)
        {
            ADD(st_null,                EvRun,              OnEvRun,            st_waitDssCallCreated);
            ADD(st_waitDssCallCreated,  EvDssCallCreated,   OnEvDssCallCreated, st_waitAlerting);

            // no channel:
            ADD(st_waitDssCallCreated,  EvDisconnent,       OnNoChannel,        st_waitLinkDisc);

            ADD(st_waitAlerting,        EvAlerting,         Nop,                st_waitConnected);
            ADD(st_waitConnected,       EvConnected,        OnEvConnected,      _DynamicState_);     // talk begin - st_waitSelfTimer or st_waitDisconnected
            ADD(st_waitSelfTimer,       EvReleaseTalk,      OnReleaseTalk,      st_waitLinkDisc);
            ADD(st_waitDisconnected,    EvDisconnent,       Nop,                st_waitLinkDisc); // test cause num?
            ADD(st_waitLinkDisc,        EvLinkDisc,         Nop,                st_complete);

            //глушим ассерт тк незная точного момента прихода EvLinkConnect
            ADD(_AnyState_,             EvLinkConnect,      Nop,                _SameState_);
        }

    public:

        InternalFsm(OutTalkByTime& owner) : 
            InternalFsmBaseForOutCall(owner.m_logSession),
            m_owner(owner)
        {
            InitTable( FSM() );
        }

        // команда от OutTalkByTime - закрыть связь
        void ReleaseTalk()
        {
            FSM().DoEvent( EvReleaseTalk() );
        }
    };

    // -----------------------------------------------------------------------

    OutTalkByTime::OutTalkByTime(ISubsToUserCall& subs, int talkTime, const NObjSimulLiveSettings& m_prof)
        : OutgoingDssUserCall(subs),
        m_logSession(subs.getLogger(), this),
        m_fsm( new InternalFsm(*this) ),
        m_tTalkTimer(this, &OutTalkByTime::OnDiscByTimer),
        m_talkTime(talkTime),
        m_statistic(m_prof.Statistic)
    {
        SetFsm( m_fsm.get() );
    }

    void OutTalkByTime::DrawStat()
    {
        m_statistic.IncrTalkCall(m_talkTime);
    }

    void OutTalkByTime::OnDiscByTimer(iCore::MsgTimer*)
    {
        m_fsm->ReleaseTalk();
    }

    bool OutTalkByTime::TryStartTimer()
    {
        if (m_talkTime == 0)
            return false;
        m_tTalkTimer.Start(m_talkTime);        
        return true;
    }

    // -----------------------------------------------------------------------

    class InTalkByTime::InternalFsm : public InternalFsmBaseForInCall
    {
        typedef InternalFsm T;

        InTalkByTime& m_owner;

        void OnEvIncoming(const EvIncomingCall& ev)
        {
            ISubsToUserCall& subs = m_owner.getSubs();            
            ESS_ASSERT(ev.m_pParams->GetCalled().Digits() == subs.getNumReceiver().Digits());
            TUT_ASSERT(ev.m_pParams->GetReqBchansCount() == 1);
            Link& link = m_owner.getLinkToDssCall();

            ev.m_dssCallLink->Connect(link); // link connect                           
        }

        void OnLinkConnect(const EvLinkConnect&)
        {
            Link& link = m_owner.getLinkToDssCall();

            link->SetLink( m_owner.getMyBinder() );

            link->AddressComplete(); // разрешаем завершить Setup
            link->Alerting();        // КПВ
            link->ConnectRsp();      // Принимаем вызов 
        }

        void OnEvConnectCnf(const EvConnectCnf&)
        {
            if ( m_owner.TryStartTimer() )
            {
                FSM().SetState(st_waitSelfTimer);
                m_owner.DrawStat(); // соединение установленно, длительность разговора известна
            }
            else
                FSM().SetState(st_waitDisconnected);
        }

        void OnEvReleaseTalk(const EvReleaseTalk&)
        {
            Writer( *getLog() ).Write() << "\nOnEvReleaseTalk ";
            ISubsToUserCall& subs = m_owner.getSubs();
            Link& link = m_owner.getLinkToDssCall();

            shared_ptr<const DssCause> cause (
                DssCause::Create(m_owner.getStack().GetL3Ptr(), IeConstants::NormCallClearing) );//TODO causeNum ???
            m_owner.getLinkToDssCall()->Disconnent( cause );
            m_owner.getLinkToDssCall().Disconnect(); // блокировка линка
        }

        void OnNoChannel(const EvDisconnent& ev)
        {
            Writer( *getLog() ).Write() << "Event disc " << ev.m_cause->ToString();

            /*if (ev.m_cause->GetCauseNum() == 16)
            TUT_ASSERT (
                ev.m_cause->GetCauseNum() == ISDN::IeConstants::NoCircChanAvailable ||
                ev.m_cause->GetCauseNum() == ISDN::IeConstants::NoRequestedCircChanAvailable);*/
        }

        void InitTable(Fsm &fsm)
        {
            ADD(st_null,                EvRun,              Nop,                st_waitIncommingCall);
            ADD(st_waitIncommingCall,   EvIncomingCall,     OnEvIncoming,       st_waitLinkConnected);
            ADD(st_waitLinkConnected,   EvLinkConnect,      OnLinkConnect,      st_waitConnConfirm);
            ADD(st_waitConnConfirm,     EvConnectCnf,       OnEvConnectCnf,     _DynamicState_);
            ADD(st_waitConnConfirm,     EvDisconnent,       OnNoChannel,        st_waitLinkDisc);
            ADD(st_waitConnConfirm,     EvDssCallErrorMsg,  Nop,                st_waitLinkDisc);
            ADD(st_waitSelfTimer,       EvReleaseTalk,      OnEvReleaseTalk,    st_waitLinkDisc);
            ADD(st_waitDisconnected,    EvDisconnent,       Nop,                st_waitLinkDisc);  // test cause num?
            ADD(st_waitLinkDisc,        EvLinkDisc,         Nop,                st_complete);
        }

    public:

        InternalFsm(InTalkByTime& owner) : 
            InternalFsmBaseForInCall(owner.m_logSession),
            m_owner(owner)
        {
            InitTable( FSM() );
        }

        void ReleaseTalk()
        {
            FSM().DoEvent( EvReleaseTalk() );
        }
    };

    // -------------------------------------------------------------------

    InTalkByTime::InTalkByTime(ISubsToUserCall& subs, int talkTime, const NObjSimulLiveSettings& m_prof)
        : IncommingDssUserCall(subs),
        m_logSession(subs.getLogger(), this),
        m_fsm( new InternalFsm(*this) ),
        m_tTalkTimer(this, &InTalkByTime::OnDiscByTimer),
        m_talkTime(talkTime),
        m_statistic(m_prof.Statistic)
    {
        SetFsm( m_fsm.get() );
    }

    void InTalkByTime::OnDiscByTimer(iCore::MsgTimer*)
    {
        m_fsm->ReleaseTalk();
    }

    void InTalkByTime::DrawStat()    
    {
        m_statistic.IncrTalkCall(m_talkTime);
    }

    bool InTalkByTime::TryStartTimer()
    {
        if (m_talkTime == 0)
            return false;
        m_tTalkTimer.Start(m_talkTime);
        return true;
    }


} // IsdnTest



/*

template declaration:
class Out xxx::InternalFsm
        : public InternalFsmBaseForOutCall<InternalFsm>
    {
        friend StateMachine<InternalFsm>;

        Out xxx& m_owner;

        struct FsmTransitionTable : boost::mpl::vector1 <
            ADD(st_null, EvRun, &T:: xxx, st_ xxx>
        > {};

        void Drop(shared_ptr<const DssCause>, IncomingDssUserCall&) //override
        {
            //
        }

        void OpposideDropped(shared_ptr<const DssCause>) //override
        {
            //
        }

    public:

        InternalFsm(Out xxx& owner, Logger* pLog)
            : InternalFsmBaseForOutCall(pLog),
            m_owner(owner)
        {}
    };

    //Owner constructor
    Out xxx::Out xxx(ISubsToUserCall& subs, IsdnStack& stack)
        : OutgoingDssUserCall(subs),
        m_fsm( new InternalFsm(*this, subs.getLogger() ) ),
        m_stack(stack)
    {
        SetFsm( m_fsm.get() );        
    }

    //---------------------------------------------------------------------------

    class In xxx ::InternalFsm
        : public InternalFsmBaseForInCall<InternalFsm>
    {
        friend StateMachine<InternalFsm>;

        In xxx & m_owner;

        struct FsmTransitionTable : boost::mpl::vector1 <
            ADD(st_null, EvRun, &T:: xxx, st_ xxx>
        > {};

        void Drop(shared_ptr<const DssCause>, OutgoingDssUserCall&) //override
        {
            //
        }

        void OpposideDropped(shared_ptr<const DssCause>) //override
        {
            //
        }

    public:

        InternalFsm(In xxx& owner, Logger* pLog)
            : InternalFsmBaseForInCall(pLog),
            m_owner(owner)
        {}
    };

    //Owner constructor
    In xxx ::In xxx (ISubsToUserCall& subs, IsdnStack& stack)
        : IncommingDssUserCall(subs),
        m_fsm( new InternalFsm( *this, subs.getLogger() ) ),
        m_stack(stack)
    {
        SetFsm( m_fsm.get() );
    }

*/


