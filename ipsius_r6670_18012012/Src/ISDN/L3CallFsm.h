#ifndef L3CALLFSM_H
#define L3CALLFSM_H

#include "stdafx.h"
// #include "Utils/StateMachine.h"
#include "Utils/Fsm.h"
#include "Utils/FullEnumResolver.h"
#include "IsdnUtils.h"
#include "iCore/MsgTimer.h"
#include "iCore/MsgObject.h"
#include "IIsdnLog.h"

#include "IIsdnL3Internal.h"
#include "IL3Call.h"
#include "DssInCallParams.h"
#include "L3Packet.h"
#include "DssUpIntf.h"
#include "DssTimer.h"
#include "L3CallFsmError.h"



namespace ISDN
{      
    class PacketAlerting;
    class PacketConnect;
    class PacketSetup;
    class PacketSetupAck;
    class PacketProceeding;
    class PacketConnectAck;
    class PacketDisc;
    class PacketRelease;
    class PacketReleaseComplete;
    class PacketInfo;
    class PacketProgress;
    //...
    class L3Call;

    class IeChannelIdentification;

    class CallRef;    

    // ---------------------------------------------------------------------

    class L3CallFsmEv
    {
    public:

        typedef CallFsmBaseEvent BaseEvent;

        class EvSetupReq            : public BaseEvent {};
        class EvProceedingReq       : public BaseEvent {};
        class EvSetupAck            : public BaseEvent {};
        class EvAlertingReq         : public BaseEvent {};
        class EvSetupRsp            : public BaseEvent {}; // == ConnectRsp
        class EvConnectAck          : public BaseEvent {}; // запрос отправки ConnectAck

        class EvReleaseReq  : public BaseEvent 
        {
        public:
            EvReleaseReq(){}
            EvReleaseReq(shared_ptr<const DssCause> cause) : m_cause(cause) {}
            shared_ptr<const DssCause> m_cause;
        };

        class EvDiscReq  : public BaseEvent 
        {
        public:
            EvDiscReq(shared_ptr<const DssCause> cause) : m_cause(cause) {}
            shared_ptr<const DssCause> m_cause;
        };

        class EvRelComplete : public BaseEvent 
        {
        public:
            EvRelComplete(shared_ptr<const DssCause> cause) : m_cause(cause) {}
            shared_ptr<const DssCause> m_cause;
        };

        class EvSetupComplete       : public BaseEvent {};
        class EvMoreInfoReq         : public BaseEvent {};

        // Expire Timer events
        class EvTimer : public BaseEvent
        {
        public:
            IDssTimer* pTimer;
            EvTimer(IDssTimer* p) : pTimer(p) {}
        };

        #define TMR(name) class name : public EvTimer { public: name(IDssTimer* p) : EvTimer(p) {} }

        TMR(EvT301);
        TMR(EvT302);
        TMR(EvT303);
        TMR(EvT304);
        TMR(EvT305);
        TMR(EvT308);
        TMR(EvT310);
        TMR(EvT313);

        #undef TMR
        
    };

    // ---------------------------------------------------------------------

    // —осто€ни€ внутренней fsm L3Call
    class L3CallFsmStates : public QObject    
    {
        Q_OBJECT;
        Q_ENUMS(States);

    protected:

        L3CallFsmStates(){}
            
    public:

        enum States
        {            
            st_null = 0,
            CallInitiated_1 = 1,
            OverlapSending_2 = 2,
            CallPresent_6 = 6,
            CallReceive_7 = 7,
            OutCallProceeding_3 = 3,
            CallDelivered_4 = 4,
            ConnectReq_8 = 8,
            InCallProc_9 = 9,
            Active_10 = 10,
            DiscRequested_11 = 11,
            DiscIndication_12 = 12,
            ReleaseReq_19 = 19,
            OverlapReceiv_25 = 25,
            tmp,
        };

        std::string ResolveState(States state) const
        {
            return Utils::EnumResolve(state);
        }
    };
   
    // ---------------------------------------------------------------------

    // ¬нутренн€€ fsm L3Call
    class L3CallFsm : 
        Utils::FsmSpecState,  // for easy enum access
        public ILoggable,
        L3CallFsmStates,      // states
        L3CallFsmEv,          // events
        boost::noncopyable
    {
    
        typedef L3CallFsm Here;
        typedef L3CallFsmStates St;

        typedef L3CallFsm T;
        typedef Utils::Fsm<BaseEvent, States> Fsm;

        struct LogRecordKinds
        {
            LogRecordKinds(ILoggable& session)
                : ChangeState(session.RegNewTag("Change State")),
                Event(session.RegNewTag("Fsm Event")),
                UnxepEvent(session.RegNewTag("Fsm Unexpected Event")),
                ReceivePacket(session.RegNewTag("Fsm receive packet")),
                PacketExchange(session.RegNewTag("Packet Exchange l2<-->l3")),
                Warning(session.RegNewTag("Warning!"))
            {}

            iLogW::LogRecordTag ChangeState;
            iLogW::LogRecordTag Event;
            iLogW::LogRecordTag UnxepEvent;
            iLogW::LogRecordTag ReceivePacket;
            iLogW::LogRecordTag PacketExchange;
            iLogW::LogRecordTag Warning;
        };

        // fields
        Fsm m_fsm;
        Utils::FsmTable m_tDownstate, 
                        m_tDatastate, 
                        m_tExpireTimer;

        IIsdnL3Internal& m_IL3;
        L3Call* m_pOwner; // IL3Call??
        IL3Call* m_pIOwner; 

        // ref on fields L3Call
        shared_ptr<const DssInCallParams> &m_pInParams;
        shared_ptr<const DssCallParams>  &m_pOutParams;
        ObjLink::ObjectLink<IDssCallEvents> &m_linkToUserCall;                  
        LogRecordKinds m_logTags;
        const bool m_isUserSide; // if false - network side        
        shared_ptr<const DssCause> m_cause; // буфер причины (TODO stack?) пока заполнен при RxDisc
        
        // не лучшее решение. так дл€ того чтоб сосредоточить логику назначени€ в дистрибьютере
        shared_ptr<IeChannelIdentification> m_IeChanForInquiry;        // при отправке Setup
        shared_ptr<IeChannelIdentification> m_IeChanForFirstResponse; //создаетс€ при приеме Setup дистрибьютером каналов
      
        // timers
        StopDssTimersGroup StopAllTimers; // функтор, знающий список таймеров
        DssTimer<Here, EvT301> T301;
        DssTimer<Here, EvT302> T302;
        DssTimer<Here, EvT303> T303;
        DssTimer<Here, EvT304> T304;
        DssTimer<Here, EvT305> T305;
        DssTimer<Here, EvT308> T308;
        DssTimer<Here, EvT310> T310; 
        DssTimer<Here, EvT313> T313; 

        // обработчики таймеров
        void ExpireT301(const EvT301&);
        void ExpireT302(const EvT302&);
        void ExpireT303(const EvT303&);
        void ExpireT304(const EvT304&);
        void ExpireT305(const EvT305&);
        void ExpireT308(const EvT308&);
        void ExpireT310(const EvT310&);        
        void ExpireT313(const EvT313&);        

        // обработчики нисход€щих сообщений 
        void DoSetupReq(const EvSetupReq&);
        void DoProceedingReq(const EvProceedingReq&);
        void DoAlertingReq(const EvAlertingReq&);
        void DoSetupResponse(const EvSetupRsp&);        
        void DoConnectAck(const EvConnectAck&);
        void DoDiscReq( const EvDiscReq& );
        void DoReleaseReq(const EvReleaseReq&);

        void DoRelCompete(const EvRelComplete& ev);  
        void DoSetupComplete(const EvSetupComplete& ev);
        void DoInfoReq(const EvMoreInfoReq& ev);

        void DoStatusSend(IeConstants::CauseNum num);

        // обработчики восход€щих событий
        void RxAlerting(const PacketAlerting& pack);
        void RxConnect(const PacketConnect& pack);
        void RxConnectAck(const PacketConnectAck& pack);
        void RxSetupIn(const PacketSetup& pack);
        void RxProceeding(const PacketProceeding& pack);
        void RxDisconnect(const PacketDisc& pack);
        void RxRelease(const PacketRelease& pack);
        void RxRelComplete(const PacketReleaseComplete& pack);
        void RxInfo(const PacketInfo& pack);
        void RxProgress(const PacketProgress& pack);
        void RxSetupAck(const PacketSetupAck& pack);
        
        // методы логировни€
        void LogEvent(const std::string& msg)
        {            
            if (getTraceOn())
                LogWriter(this, m_logTags.Event).Write()  << "event " << msg 
                << " in state " << ResolveState( m_fsm.GetState() );            
        }

        void OnStateChange(States prevState, States newState)
        {
            if ( getTraceOn() )
            {
                LogWriter(this, m_logTags.ChangeState).Write() << "change state from " 
                    << ResolveState(prevState)                 
                    << " to state " << ResolveState(newState);
            }
                
            WriteTalkStat(prevState, newState);            
        }

        void WriteTalkStat(States prevState, States newState);
       
        void OnNoEventHandle(const BaseEvent &e)
        {
            std::stringstream ss;
            ss << "Unexpected Event " << typeid(e).name();
            LogEvent( ss.str() );

            if (CurrStateIsNull()) return;

            const L3Packet *p = dynamic_cast<const L3Packet*>(&e);
            if (p == 0) return;

            if (m_linkToUserCall.Connected())                             
            {
                shared_ptr<const CallWarning> warn( 
                    UnexpectedPackReceived::Create( m_IL3.GetInfra(), p->GetAsString(true) ) 
                    );

                m_linkToUserCall->DssCallErrorMsg(warn);
                return;
            }

            LogEvent("ASSERT! ObjLink not connected!");                                         
        }

        void Send(const L3Packet& pack)
        {            
            if (getTraceOn() && m_IL3.getTraceOption().m_traceDataExcangeL3Call)
                LogWriter(this, m_logTags.PacketExchange).Write() 
                << "Send packet:\n" << pack.GetAsString(m_IL3.getTraceOption().m_traceIeContent);

            m_IL3.Send(pack);
        }

        template<class TEvent>
        void ProcessCCevent(const TEvent& ev)
        {
            /*
            if (m_isUserSide)
                ProcessEventEx<TEvent, DownstateUser>(ev);
            else
                ProcessEventEx<TEvent, DownstateNetwork>(ev); */

            m_fsm.DoEvent(ev, m_tDownstate);
        }

        template<class TEvent>
        void ProcessTimerEvent(const TEvent& ev)
        {
            /*
            if (m_isUserSide)
                ProcessEventEx<TEvent, ExpireTimerUser>(ev);
            else
                ProcessEventEx<TEvent, ExpireTimerNet>(ev); */

            m_fsm.DoEvent(ev, m_tExpireTimer);
        }

        template<class TWarning>
        void SendWarning()
        {
            shared_ptr<const CallWarning> pWarning(TWarning::Create( m_IL3.GetInfra() ));
            if (m_linkToUserCall.Connected())
            {
                m_linkToUserCall->DssCallErrorMsg(pWarning);
                return;
            }

            if (getTraceOn())
            {
                LogWriter(this, m_logTags.Warning).Write() 
                    << "Can`t send warning because link disconnected.";
            }
        }
        
        void SendTimeoutInd(const std::string& msg)
        {            
            if (m_linkToUserCall.Connected())                            
                m_linkToUserCall->DssCallErrorMsg(
                shared_ptr<const CallWarning>(TimeoutIndication::Create(m_IL3.GetInfra(), msg))
                );                         
        }
        
        void ProcessChanResponse(const L3Packet& pack); // if procedure failed can throw ErrActionCloseCall
        void ProcessSetupAck(const L3Packet& pack); // if procedure failed can throw ErrActionCloseCall        

        void InitFsmTableUser(Fsm &fsm);
        void InitFsmTableNetwork(Fsm &fsm);

    public:
        L3CallFsm(IIsdnL3Internal& IL3, L3Call *pOwner, shared_ptr<const DssInCallParams>& pInParams,
            shared_ptr<const DssCallParams>& pOutParams, ObjLink::ObjectLink<IDssCallEvents>& pL4,
            const CallRef* cref);

        ~L3CallFsm()
        {
            StopAllTimers();
        }

        bool CurrStateIsNull() const { return m_fsm.GetState() == st_null; }

        //bool CurrStateIsTalk() { return getCurrentState() == Active_10; }

        template<class TPacket>
        void RoutePacket(TPacket *pPacket)
        {             
            if (getTraceOn())
            {
                LogWriter(this, m_logTags.ReceivePacket).Write() << pPacket->GetName() 
                    << " in state " 
                    << ResolveState( m_fsm.GetState() );

                const DssTraceOption& option = m_IL3.getTraceOption();

                if (option.m_traceDataExcangeL3Call)
                    LogWriter(this, m_logTags.PacketExchange).Write() << "Received packet:\n"
                    << pPacket->GetAsString(option.m_traceIeContent);
            }
            
            try
            {
                /*
                if (m_isUserSide)
                    ProcessEventEx<TPacket, DatastateUser>(*pPacket);
                else
                    ProcessEventEx<TPacket, DatastateNetwork>(*pPacket); */

                m_fsm.DoEvent(*pPacket, m_tDatastate);
            }
            catch(const ErrPacketProcess& e)
            {                
                ProcessException(e);
            }            
        }

        // обработка ошибок возникших в табличных обработчиках пакетов
        void ProcessException(const ErrPacketProcess& e);

        void CCReleaseComplete(shared_ptr<const DssCause> pCause)
        {
            //ESS_ASSERT(0);//TODO
            LogEvent("SendRelComplete");
            ProcessCCevent( EvRelComplete(pCause) );
        }
               
        void CCSetupReq() // override
        {            
            LogEvent("SetupReq");
            ProcessCCevent( EvSetupReq() );            
        }

        void CCProceedingReq()
        {
            /*if (getCurrentState() == OverlapSending_2)
                std::cout << "Break" << std::endl;*/
            LogEvent("ProceedingReq");
            ProcessCCevent( EvProceedingReq() );
        }

        void CCAlertingReq()
        {
            LogEvent("AlertingReq");
            ProcessCCevent( EvAlertingReq() );
        }

        void CCSetupRsp()
        {
            LogEvent("SetupResponse(called connected)");
            ProcessCCevent( EvSetupRsp() );
        }

        void CCDiscReq(  shared_ptr<const DssCause> pCause )
        {            
            LogEvent("DiscReq with cause " + pCause->ToString());
            ProcessCCevent( EvDiscReq(pCause) );           
        }

        void CCSetupComplete()
        {
            LogEvent("SetupComplete");
            ProcessCCevent( EvSetupComplete() );
        }

        void CCMoreInfoReq()
        {
            LogEvent("MoreInfoReq");
            ProcessCCevent( EvMoreInfoReq() );
        }

        void DoCloseCall(IeConstants::CauseNum num)  // called from ErrActionCloseCall
        {
            m_fsm.SetState(st_null);
            m_pIOwner->CloseCall( DssCause::Create(&m_IL3, num) );
        }

    private://TODO

        void CCReleaseReq()
        {
            LogEvent("ReleaseReq");
            ProcessCCevent( EvReleaseReq() );
        }

        void CCSetupAck()
        {            
            LogEvent("SetupAck");
            ProcessCCevent( EvSetupAck() );
        }  
        
    };

} // ISDN

#endif

