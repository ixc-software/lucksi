#ifndef USERCALLINTERNALFSMBASE_H
#define USERCALLINTERNALFSMBASE_H

#include "Utils/Fsm.h"
#include "Utils/QtEnumResolver.h"

#include "ISDN/IsdnUtils.h"

#include "DssUserCall.h"
#include "logger.h"



namespace IsdnTest
{

    using Utils::QtEnumResolver;    
    using ISDN::BinderToDssCall;    
    using ISDN::IDssCall;
    using boost::shared_ptr;
    using ISDN::SetBCannels;
    using ISDN::CallWarning;
    using ISDN::DssCause;
    //using ISDN::IsdnUtils::FinalName;
    using ISDN::IsdnUtils::TypeToShortName;

    

    // ����������� ������� ���������� ��� TODO ��������� In/Out ��� ����� � ���� ����� - ?
    class InUserCallInternalEvents
    {
    protected:

        class BaseEvent
        {
        public:
            virtual ~BaseEvent() {}
        };

        class EvConnectCnf : public BaseEvent {};

        class EvDssCallErrorMsg : public BaseEvent 
        {
        public:
            shared_ptr<const CallWarning> m_err;
            EvDssCallErrorMsg( shared_ptr<const CallWarning> err) : m_err(err) {}
        };

        class EvDisconnent : public BaseEvent 
        {
        public:
            shared_ptr<const DssCause> m_cause;
            EvDisconnent(shared_ptr<const DssCause> cause) : m_cause(cause) {}
        };

        class EvRun : public BaseEvent {};

        class EvIncomingCall : public BaseEvent 
        {
        public:

            BinderToDssCall m_dssCallLink;
            shared_ptr<const DssCallParams> m_pParams;

            EvIncomingCall(BinderToDssCall dssCall, shared_ptr<const DssCallParams> pParams)
                : m_dssCallLink(dssCall),
                m_pParams(pParams)
            {}
        };

        // ��������� � ������� �����
        class EvLinkDisc  : public BaseEvent {};

        // ���� ����������
        class EvLinkConnect :  public BaseEvent {};

        class EvReleaseTalk :  public BaseEvent {};

    };

    //-------------------------------------------------------------------------------

    // ������� ��� ���������� fsm ��������� �������
    class OutUserCallInternalEvents
    {
    protected:

        class BaseEvent 
        {
        public:
            virtual ~BaseEvent() {}
        };

        class EvDssCallErrorMsg : public BaseEvent
        {
        public:
            shared_ptr<const CallWarning> m_err;
            EvDssCallErrorMsg( shared_ptr<const CallWarning> err) : m_err(err) {}
        };

        class EvDisconnent : public BaseEvent
        {
        public:
            shared_ptr<const DssCause> m_cause;
            EvDisconnent(shared_ptr<const DssCause> cause) : m_cause(cause) {}
        };

        class EvDssCallCreated : public BaseEvent
        {
        public:
            const ISDN::IDssCallEvents::DssCallCreatedParametrs &m_params;

            EvDssCallCreated(const ISDN::IDssCallEvents::DssCallCreatedParametrs &p) : m_params(p) {}
        };

        class EvAlerting       : public BaseEvent {};
        class EvConnected      : public BaseEvent {};
        class EvConnectCnf     : public BaseEvent {};
        
        class EvLinkDisc       : public BaseEvent {};  // ��������� � ������� �����        
        class EvLinkConnect    : public BaseEvent {}; // ���� ����������

        class EvRun            : public BaseEvent {};
        class EvReleaseTalk    : public BaseEvent {};
    };

    //-------------------------------------------------------------------------------

    // ������������ ��������� ���������� ��� TODO ��������� �� In/Out?
    class UserCallInternalStates : public QObject    
    {
        Q_OBJECT;
        Q_ENUMS(St);

    public:

        enum St
        {
            st_null,
            st_waitDssCallCreated,
            st_waitConnConfirm,
            st_waitIncommingCall,
            st_waitAlerting,
            st_waitConnected,
            st_waitDisconnected,        // �������� ������� �� DSsCall � ���������� ������
            st_waitDiscWithCause,       // �������� Disconnected � ����������� ��������
            st_waitLinkDisc,            // �������� ����������� � ������� �����
            st_waitLinkConnected,       // �������� ������������� ��������� �����
            st_waitDssCallErrorMsg,
            st_waitSelfTimer,           // �������� ������� �� ������ �������
            st_complete,
        };

    };

    // ----------------------------------------------------------------------

    struct LogTagsForUserCallFsm
    {
        LogTagsForUserCallFsm(Logger& session)
            : Event(session.RegNewTag("Event")),
            ChangeState(session.RegNewTag("Change state")),
            NoTransition(session.RegNewTag("Warning! Unexpected event"))
        {}

        iLogW::LogRecordTag Event;
        iLogW::LogRecordTag ChangeState;
        iLogW::LogRecordTag NoTransition;
    };

    // ----------------------------------------------------------------------

    // ���� ��� ���������� fsm �������� �������
    class InternalFsmBaseForInCall : 
        public IFsmIncomming,
        public UserCallInternalStates,      // states 
        public InUserCallInternalEvents,    // events 
        protected Utils::FsmSpecState,      // for enums 
        boost::noncopyable
    {

    protected:

        typedef Utils::Fsm<BaseEvent, St> Fsm;

    private:        

        typedef InternalFsmBaseForInCall T;

        Fsm m_fsm;
        Logger &m_logSession;
        LogTagsForUserCallFsm m_tags;
        QtEnumResolver<UserCallInternalStates, UserCallInternalStates::St> m_resolver;
        
        void ProcEv(const BaseEvent &ev)
        {
            Writer(m_logSession, m_tags.Event).Write() 
                << Platform::FormatTypeidName( typeid(ev).name() );

            m_fsm.DoEvent(ev);
        }

    // IFsmIncomming implementation
    private:

        void OnConnectConfirm() 
        {
            ProcEv( EvConnectCnf() );
        }

        void OnDssCallErrorMsg( shared_ptr<const CallWarning> e)
        {
            ProcEv( EvDssCallErrorMsg(e) );
        }

        void OnDisconnent( shared_ptr<const DssCause> cause)
        {
            ProcEv( EvDisconnent(cause) );
        }

        void OnLinkDisc()
        {
            ProcEv( EvLinkDisc() );
        }

        void OnLinkConnect()
        {
            ProcEv( EvLinkConnect() );
        }

        void OnRun()
        {
            ProcEv( EvRun() );
        }

        void OnIncommingCall(BinderToDssCall dssCall, shared_ptr<const DssCallParams> pParams)
        {
            ProcEv( EvIncomingCall(dssCall, pParams) );
        }

        bool StateIsComplete() 
        {
            return m_fsm.GetState() == st_complete;
        }

    protected:

        void OnStateChange(St prevState, St newState)
        {
            if (prevState == st_complete) ESS_ASSERT(prevState != st_complete);

            Writer(m_logSession, m_tags.ChangeState).Write() << "from " 
                << StateToString(prevState) << " to state " << StateToString(newState);            
        }

        void OnNoEventHandle(const BaseEvent &e)
        {
            std::string s = "NoEventHandle: ";
            s += Platform::FormatTypeidName( typeid(e).name() );
            s += " in state " + StateToString( m_fsm.GetState() );

            Writer(m_logSession, m_tags.NoTransition).Write() << s;

            // special case for EvLinkConnect
            {
                const EvLinkConnect *pEv = dynamic_cast<const EvLinkConnect*>(&e);
                if (pEv != 0) return;
            }

            // stop
            std::cout << s << std::endl;
            TUT_ASSERT(0 && "Unexpected Event in usercall fsm!");
        }

        
    protected:

        InternalFsmBaseForInCall(Logger& logSession) : 
            m_fsm(st_null, false, boost::bind(&T::OnNoEventHandle, this, _1),
                                  boost::bind(&T::OnStateChange, this, _1, _2) ),
            m_logSession(logSession),
            m_tags(logSession)
        {
        }

        Logger* getLog()
        {
            return &m_logSession;
        }

        std::string StateToString(St state)
        {
            return m_resolver.Resolve(state);
        }

        Fsm& FSM() { return m_fsm; }

        void Nop(const BaseEvent&) {}

    };

    //-------------------------------------------------------------------------------

    // ���� ��� ���������� fsm ��������� �������
    class InternalFsmBaseForOutCall : 
        public IFsmOutgoing,
        public UserCallInternalStates,      // states 
        public OutUserCallInternalEvents,   // events
        protected Utils::FsmSpecState,      // for enum
        boost::noncopyable
    {       
    protected:
        
        typedef Utils::Fsm<BaseEvent, St> Fsm;
        // typedef StateMachine<TDrived> Base;
        
    private:

        typedef InternalFsmBaseForOutCall T;

        Fsm m_fsm;
        Logger &m_logSession;
        LogTagsForUserCallFsm m_tags;
        QtEnumResolver<UserCallInternalStates, UserCallInternalStates::St> m_resolver;
       
        void ProcEv(const BaseEvent &ev)
        {
            Writer(m_logSession, m_tags.Event).Write() 
                << Platform::FormatTypeidName( typeid(ev).name() );

            m_fsm.DoEvent(ev);
        }

    // IFsmOutgoing implementation
    private: 

        void OnAlerting() 
        {
            ProcEv( EvAlerting());
        }

        void OnConnected() 
        {
            ProcEv( EvConnected() );
        }

        void OnConnectConfirm()
        {
            ProcEv( EvConnectCnf() );
        }

        void OnDssCallCreated(ISDN::IDssCallEvents::DssCallCreatedParametrs e) 
        {            
            ProcEv( EvDssCallCreated(e) );
        }

        void OnDssCallErrorMsg( shared_ptr<const CallWarning> e)
        {
            ProcEv( EvDssCallErrorMsg(e) );
        }

        void OnDisconnent( shared_ptr<const DssCause> cause)
        {
            ProcEv( EvDisconnent(cause) );
        }

        void OnLinkDisc()
        {
            ProcEv( EvLinkDisc() );
        }

        void OnLinkConnect()
        {
            ProcEv( EvLinkConnect() );
        }

        void OnRun()
        {
            ProcEv( EvRun() );
        }

        bool StateIsComplete() 
        {
            return m_fsm.GetState() == st_complete;
        }
        
    protected:

        void OnStateChange(St prevState, St newState)
        {
            Writer(m_logSession, m_tags.ChangeState).Write() << "from " 
                << StateToString(prevState) << " to state " << StateToString(newState);            
        }
        
        void OnNoEventHandle(const BaseEvent &e)
        {
            std::string s = "NoEventHandle: ";
            s += Platform::FormatTypeidName( typeid(e).name() );
            s += " in state " + StateToString( m_fsm.GetState() );

            Writer(m_logSession, m_tags.NoTransition).Write() << s;

            // special case for EvLinkConnect
            {
                const EvLinkConnect *pEv = dynamic_cast<const EvLinkConnect*>(&e);
                if (pEv != 0) return;
            }

            // stop
            std::cout << s << std::endl;
            TUT_ASSERT(0 && "Unexpected Event in usercall fsm!");
        }
        
    protected:

        InternalFsmBaseForOutCall(Logger& logSession) :
             m_fsm(st_null, false, boost::bind(&T::OnNoEventHandle, this, _1), 
                                   boost::bind(&T::OnStateChange, this, _1, _2) ),
            m_logSession(logSession),
            m_tags(logSession)
        {
        }

        Logger* getLog()
        {
            return &m_logSession;
        }

        std::string StateToString(St state)
        {
            return m_resolver.Resolve(state);
        }

        Fsm& FSM() { return m_fsm; }

        void Nop(const BaseEvent&) {}

    };

} // IsdnTest

#endif

