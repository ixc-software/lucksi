#include "stdafx.h"
#include "UserCallForYate.h"
#include "Utils/StateMachine.h"

#include "YateL3TestProfile.h"



namespace IsdnTest
{
    // ожидает входящий вызов от Yate-Dss. Принимает, обрывает через некоторое время
    class UserCallForYate::FsmIncoming : public Utils::StateMachine<UserCallForYate::FsmIncoming>
    {        
        friend class Utils::StateMachine<UserCallForYate::FsmIncoming>;
        typedef UserCallForYate::EvLinkConnect EvLinkConnect;
        

        UserCallForYate& m_shell;

        enum State
        {
            st_null,
            st_callReceived,
            st_talk,
            st_complete,
            _FsmInitialState_ = st_null
        };

        template<class TEv>
        void Nop(const TEv&)
        {}

        void OnLinkConnect(const EvLinkConnect&)
        {
            m_shell.SendLink();

            m_shell.m_link->Alerting();
            m_shell.m_link->ConnectRsp();
            
        }

        void OnConnectCnf(const EvConnectCnf&)
        {
            m_shell.m_tDisc.Start(12000);
        }

        void DoDisc(const EvDiscTimer&)
        {
            //ужас
            shared_ptr<const ISDN::DssCause> cause (
                ISDN::DssCause::Create(
                    m_shell.m_owner.getInfra(), m_shell.m_owner.getProf().getLocation(), 
                    ISDN::IeConstants::CCITT, ISDN::IeConstants::NormCallClearing
                ) 
            );

            m_shell.m_link->Disconnent(cause);
            m_shell.m_link.Disconnect();
        }

        struct FsmTransitionTable : boost::mpl::vector3<
            EvtRow<st_null, EvLinkConnect, &T::OnLinkConnect, st_callReceived>,
            EvtRow<st_callReceived, EvConnectCnf, &T::OnConnectCnf, st_talk>,
            EvtRow<st_talk, EvDiscTimer, &T::DoDisc, st_complete>
        >{};

        void OnStateChange(int prevState, int newState)
        {
            Writer(m_shell.m_logSession).Write() << "Change State from " << prevState << " to " << newState;
        }

        template<class TEvent>
        void OnNoEventHandle(const TEvent &e)
        {
            Writer(m_shell.m_logSession).Write() << "NoEventHandle event: " << typeid(TEvent).name();
        }

    public:

        FsmIncoming(UserCallForYate& shell)
            : m_shell(shell)
        {}

        bool IsComplete()
        {
            return getCurrentState() == st_complete;
        }
    };

    //-------------------------------------------------------------------------------------------------------

    // Установка соединения, ожидание разрыва
    class UserCallForYate::FsmOutgoing : public Utils::StateMachine<UserCallForYate::FsmOutgoing>
    {        
        friend class Utils::StateMachine<UserCallForYate::FsmOutgoing>;
        typedef UserCallForYate::EvLinkConnect EvLinkConnect;


        UserCallForYate& m_shell;

        enum State
        {
            st_waitCallCreted,
            st_waitLinkConnect,
            st_waitAlerting,
            st_waitConnected,
            st_waitDisc,
            st_complete,
            _FsmInitialState_ = st_waitCallCreted
        };

        template<class TEv>
        void Nop(const TEv& e)
        {}

        void OnCallCreated( const EvCallCreated& params)
        {
            TUT_ASSERT(params.m_setBCannels->size() == 1);
            TUT_ASSERT(params.m_linkBinder->Connect( m_shell.m_link ));
        }

        /*void OnLinkConnect(const EvLinkConnect& ev)
        {
        }*/

        void DoDisc(const EvDiscTimer&)
        {
            //ужас
            shared_ptr<const ISDN::DssCause> cause (
                ISDN::DssCause::Create(
                m_shell.m_owner.getInfra(), m_shell.m_owner.getProf().getLocation(), 
                ISDN::IeConstants::CCITT, ISDN::IeConstants::NormCallClearing
                ) 
                );

            m_shell.m_link->Disconnent(cause);
            m_shell.m_link.Disconnect();
        }
        
        struct FsmTransitionTable : boost::mpl::vector5<
            EvtRow<st_waitCallCreted, EvCallCreated, &T::OnCallCreated, st_waitLinkConnect>,
            EvtRow<st_waitLinkConnect, EvLinkConnect, &T::Nop, st_waitAlerting>,
            EvtRow<st_waitAlerting, EvAlerting, &T::Nop, st_waitConnected>,
            EvtRow<st_waitConnected, EvConnected, &T::Nop, st_waitDisc>,
            EvtRow<st_waitDisc, EvDisc, &T::Nop, st_complete >
        >{};

        void OnStateChange(int prevState, int newState)
        {
            Writer(m_shell.m_logSession).Write() << "Change State from " << prevState << " to " << newState;
        }

        template<class TEvent>
        void OnNoEventHandle(const TEvent &e)
        {
            Writer(m_shell.m_logSession).Write() << "NoEventHandle event: " << typeid(TEvent).name();
        }

    public:

        FsmOutgoing(UserCallForYate& shell)
            : m_shell(shell)
        {}

        bool IsComplete()
        {
            return getCurrentState() == st_complete;
        }
    };   


    //=============================================================================

    UserCallForYate::UserCallForYate(IDssCallToDssUserForYate& owner, 
                                     BinderToDssCall bindToDssCall, shared_ptr<const DssCallParams> callParams)
        : MsgObject(owner.getDomain().getMsgThread()),
        m_owner(owner),
        m_logSession(owner.getLogSession(), "Incoming UserCall For Yate"),
        m_tDisc(this, &UserCallForYate::OnDiscTimer),
        m_pInFsm( new FsmIncoming(*this) ),        
        m_server(owner.getDomain(), this),
        m_domain(owner.getDomain()),
        m_link(*this)
    {
        TUT_ASSERT( bindToDssCall->Connect(m_link) );
    }

    // Outgoing call
    UserCallForYate::UserCallForYate(IDssCallToDssUserForYate& owner, shared_ptr<const DssCallParams> callParams)
        : MsgObject(owner.getDomain().getMsgThread()),
        m_owner(owner),
        m_logSession(owner.getLogSession(), "Outgoing UserCall For Yate"),
        m_tDisc(this, &UserCallForYate::OnDiscTimer),
        m_pOutFsm( new FsmOutgoing(*this) ),
        m_server(owner.getDomain(), this),
        m_domain(owner.getDomain()),
        m_link(*this)
    {
        //...
    }

    template<class TEvent>
    void UserCallForYate::EventToFsm(const TEvent& ev)
    {
        if (m_pInFsm.get() != 0)
            m_pInFsm->ProcessEvent(ev);
        else if (m_pOutFsm.get() != 0)
            m_pOutFsm->ProcessEvent(ev);
        else
            ESS_ASSERT(0 && "No initialized fsm.");
    }

    void UserCallForYate::OnDiscTimer(iCore::MsgTimer*)
    {        
        EventToFsm( EvDiscTimer() );
    }

    void UserCallForYate::Alerting()
    {
        EventToFsm( EvAlerting() );
    }

    void UserCallForYate::ConnectConfirm()
    {
        EventToFsm( EvConnectCnf() );
    }

    void UserCallForYate::Connected()
    {
        EventToFsm( EvConnected() );
    }

    void UserCallForYate::DssCallCreated(DssCallCreatedParametrs params)
    {
        EventToFsm( params );
    }

    void UserCallForYate::DssCallErrorMsg(Warning pWarning)
    {
        EventToFsm( pWarning );
    }
    
    void UserCallForYate::Disconnent(Cause pCause)
    {
        EventToFsm( pCause );
    }

    void UserCallForYate::OnObjectLinkConnect(ILinkKeyID &linkID)
    {
        EventToFsm( EvLinkConnect() );
    }

    void UserCallForYate::OnObjectLinkDisconnect(ILinkKeyID &linkID)
    {
        EventToFsm( EvLinkDisc() );
    }

    bool UserCallForYate::IsComplete()
    {
        if(m_pInFsm.get() != 0)
            return m_pInFsm->IsComplete();
        else if (m_pOutFsm.get() != 0)
            return m_pOutFsm->IsComplete();
        return false;
    }

} // IsdnTest
