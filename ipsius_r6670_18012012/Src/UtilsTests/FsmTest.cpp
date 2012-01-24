#include "stdafx.h"

#include "Utils/Fsm.h"

// --------------------------------------------------------------------

namespace X1
{

    // basic event
    class GamerEventBasic
    {
    public:
        virtual ~GamerEventBasic() {}
    };

    // events
    class GamerMove : public GamerEventBasic
    {};

    class GamerStop : public GamerEventBasic
    {};

    class GamerDie : public GamerEventBasic
    {};

    class GamerRecover : public GamerEventBasic
    {};

    // --------------------------------------

    // FsmImplement
    class Game : 
        Utils::FsmSpecState // for easy enum access
    {

        // states
        enum States
        {
            Alive,
            Dead,
            Moving,
            Stoped,
        };


        typedef Game T;
        typedef Utils::Fsm<GamerEventBasic, States> Fsm;

        Fsm m_fsm;

        // for test
        int m_wasDeadButRecoverCounter;
        int m_unionHandlerCounter;
        int m_deadCounter;
        int m_dieAlreadyCounter;
        int m_noHandlerCounter;

        // handlers
        void RiseFromDead(const GamerRecover &e)
        {
            ++m_wasDeadButRecoverCounter;
        }

        void UnionAliveAction(const GamerEventBasic &e)
        {
            ++m_unionHandlerCounter;
        }

        void StopMoving(const GamerStop &e)
        {
            TUT_ASSERT(m_fsm.GetState() == Moving);

            m_fsm.SetState(Stoped);

            TUT_ASSERT(m_fsm.GetState() == Stoped);
        }

        void BecomeMoving(const GamerMove &e)
        {
            m_fsm.SetState(Moving);

            TUT_ASSERT(m_fsm.GetState() == Moving);
        }

        void DieAlready(const GamerDie &e)
        {
            ++m_dieAlreadyCounter;

            m_fsm.SetState(Stoped);   // cause ESS_ASSERT()
        }

        void WhenDie(const GamerDie &e)
        {
            ++m_deadCounter; 
        }

        void None(const GamerStop &e)
        {
            // none
        }

        void InitTable(Fsm &fsm)
        {
            #define ADD UTILS_FSM_ADD

            ADD(Dead,       GamerRecover,   RiseFromDead,       Alive);             // (1)
            ADD(Alive,      GamerMove,      UnionAliveAction,   Moving);            // (2)
            ADD(Alive,      GamerStop,      UnionAliveAction,   Stoped);            // (3) 
            ADD(Moving,     GamerStop,      StopMoving,         _DynamicState_);    // (4)            
            ADD(Stoped,     GamerMove,      BecomeMoving,       _DynamicState_);    // (5)
            ADD(Dead,       GamerDie,       DieAlready,         _SameState_);       // (6)
            ADD(_AnyState_, GamerDie,       WhenDie,            Dead);              // (7)

            #undef ADD
        }

        // fsm helpers
        void OnNoEventHandle(const GamerEventBasic &e)
        {
            ++m_noHandlerCounter;
        }

        template<class TEvent>
        void DoEvent(const TEvent &e)
        {
            m_fsm.DoEvent(e);
        }

        void Test()
        {
            ESS::ExceptionHookRethrow<> hook;
            int countExceptions = 0;

            //(no handler) check that no-handler-in-table
            DoEvent( GamerRecover() );            // state don't change
            TUT_ASSERT(m_noHandlerCounter == 1);

            //(2) test UnionHandler
            DoEvent( GamerMove() );               // alive -> moving
            TUT_ASSERT(m_unionHandlerCounter == 1);

            //(4) check setting (correct) state by handler
            DoEvent( GamerStop() );               // moving -> stoped 
            // assertion inside 

            //(5) check setting (incorrect) state by handler -> assertion
            DoEvent( GamerMove() );               // stoped -> imposible -> moving
            // assertion inside 

            //(7) check _AnyState_
            DoEvent( GamerDie() );                // _AnyState_ -> dead 
            TUT_ASSERT(m_deadCounter == 1);

            //(6) check _AnyState_ specialization && check _SameState_ : 
            //        try to change state without access in handler -> assertion
            bool wasException = false;
            try
            {
                DoEvent( GamerDie() );            // try: dead -> stoped, but can't
            }
            catch(ESS::HookRethrow &e)
            {
                TUT_ASSERT(m_dieAlreadyCounter == 1);

                wasException = true;

                // obsolete, you can't change state to incorrect
                /*
                DoEvent( GamerDie() );            // repair chain : stoped -> dead
                TUT_ASSERT(m_deadCounter == 2);  */
            }
            TUT_ASSERT(wasException); // ok

            //(1) assert call RiseFromDead()  && change state using table
            DoEvent( GamerRecover() );            // dead -> alive 
            TUT_ASSERT(m_wasDeadButRecoverCounter == 1);

            //(2, 3) test UnionHandler
            DoEvent( GamerStop() );               // alive -> stoped
            TUT_ASSERT(m_unionHandlerCounter == 2);         

            // final check
            TUT_ASSERT(m_noHandlerCounter == 1);
            TUT_ASSERT(m_unionHandlerCounter == 2);
            TUT_ASSERT(m_deadCounter == 1);
            TUT_ASSERT(m_wasDeadButRecoverCounter == 1);
            TUT_ASSERT(m_dieAlreadyCounter == 1);
        }

    public:

        Game() : 
            m_fsm( Alive, false, boost::bind(&T::OnNoEventHandle, this, _1)  ),
            m_wasDeadButRecoverCounter(0),m_unionHandlerCounter(0),m_deadCounter(0),
            m_dieAlreadyCounter(0), m_noHandlerCounter(0)
        {            
            InitTable(m_fsm);
            Test();
        }

    };

} // namespace X1

// --------------------------------------

namespace X2
{
    // gamer events
    class GamerEventBasic
    {
    public:
        virtual ~GamerEventBasic() {}
    };

    class GamerMove : public GamerEventBasic
    {};

    class GamerStop : public GamerEventBasic
    {};

    class NetworkConnected : public GamerEventBasic
    {};

    class NetworkDisconnected : public GamerEventBasic
    {};

    // ------------------------------------------------

    class OnlineGame : 
        Utils::FsmSpecState // for easy enum access
    {

        // states
        enum States
        {
            NtConnected,
            NtDisconnected,
            GamerMoving,
            GamerStoped,
        };

        typedef OnlineGame T;
        typedef Utils::Fsm<GamerEventBasic, States> Fsm;

        Fsm m_fsm;
        Utils::FsmTable tNetwork, tLocal;
        bool m_handleNetworkEvents;

        void OnConnect(const NetworkConnected &e)
        {
            m_handleNetworkEvents = true;
        }

        void OnDisconnect(const NetworkDisconnected &e)
        {
            m_handleNetworkEvents = true;
        }

        void BecomeMoving(const GamerMove &e)
        {
            m_handleNetworkEvents = false;
        }

        void StopMoving(const GamerStop &e)
        {
            m_handleNetworkEvents = false;
        }

        void InitTable(Fsm &fsm)
        {
            #define ADD UTILS_FSM_ADD

            ADD( NtDisconnected,     NetworkConnected,     OnConnect,      NtConnected );
            ADD( NtConnected,        NetworkDisconnected,  OnDisconnect,   NtDisconnected );
            tNetwork = fsm.CloseTable();

            ADD( NtConnected,        GamerMove,            BecomeMoving,   GamerMoving );
            ADD( GamerStoped,        GamerMove,            BecomeMoving,   GamerMoving );
            ADD( GamerMoving,        GamerStop,            StopMoving,     GamerStoped );
            tLocal = fsm.CloseTable();

            #undef ADD
        }

        void Test()
        {
            // assert that we in FsmForNetwork
            m_fsm.DoEvent( NetworkConnected(), tNetwork );
            TUT_ASSERT( m_handleNetworkEvents );

            // assert that we in FsmForGamer
            m_fsm.DoEvent( GamerMove(), tLocal );
            TUT_ASSERT( !m_handleNetworkEvents );
        }

    public:

        OnlineGame() : m_fsm(NtDisconnected), m_handleNetworkEvents(false)
        {
            InitTable(m_fsm);
            Test();
        }

    };

} // namespace X2

// --------------------------------------

namespace X3
{

    class Event 
    {
    public:
        virtual ~Event() {}
    };

    class Event1 : public Event {};
    class Event2 : public Event {};
    class Event3 : public Event {};

    class Test :
        Utils::FsmSpecState // for easy enum access
    {
        enum State
        {
            st1, st2, st3, st4,
        };

        typedef Test T;
        typedef Utils::Fsm<Event, State> Fsm;

        Fsm m_fsm;
        int m_stateSwitchCounter;
        std::string m_lastMsg;

        void OnEv1(const Event1 &e)
        {
            m_lastMsg = "OnEv1";
            m_fsm.SetState(st2);
        }

        void OnEv1Bad(const Event1 &e)
        {
            TUT_ASSERT(false);
        }

        void OnEv2(const Event2 &e)
        {
            m_lastMsg = "OnEv2";
        }

        void OnEvAny(const Event &e)
        {
            m_lastMsg = "OnEvAny";
        }

        void OnAnyAny(const Event &e)
        {
            m_lastMsg = "OnAnyAny";
        }

        void NoTransition(const Event &e)
        {
            TUT_ASSERT(false);
        }

        void ChangeState(State stPrev, State stNew)
        {
            ++m_stateSwitchCounter;
        }

        void InitTable(Fsm &fsm)
        {
            #define ADD UTILS_FSM_ADD

            ADD( _AnyState_,    Event1,  OnEv1,    _DynamicState_);  // #1
            ADD( st1,           Event1,  OnEv1Bad, _DynamicState_);  // #2, overrided by #1
            ADD( st2,           Event2,  OnEv2,    st3);             // #3
            ADD( st3,           Event,   OnEvAny,  st4);             // #4
            ADD( _AnyState_,    Event,   OnAnyAny, _SameState_);     // #5

            #undef ADD
        }

        void RunTest()
        {
            m_fsm.DoEvent( Event1() );  // #1
            TUT_ASSERT(m_lastMsg == "OnEv1");

            m_fsm.DoEvent( Event2() );  // #3
            TUT_ASSERT(m_lastMsg == "OnEv2");

            m_fsm.DoEvent( Event3() );  // #4
            TUT_ASSERT(m_lastMsg == "OnEvAny");

            m_fsm.DoEvent( Event3() );  // #5
            TUT_ASSERT(m_lastMsg == "OnAnyAny");

            TUT_ASSERT(m_stateSwitchCounter == 3);
        }

    public:

        Test() : 
          m_fsm(st1, true, 
              boost::bind(&T::NoTransition, this, _1), 
              boost::bind(&T::ChangeState, this, _1, _2) ),
          m_stateSwitchCounter(0)
        {
            InitTable(m_fsm);

            RunTest();
        }

    };

}  // namespace X3

// --------------------------------------

namespace X4
{

    class Test : Utils::FsmSpecState
    {

        class Event 
        {
        public:
            virtual ~Event() {}
        };

        class Event1 : public Event {};

        enum State
        {
            st1, st2, st3, st4, st5,
        };

        typedef Test T;
        typedef Utils::Fsm<Event, State> Fsm;

        Fsm m_fsm;

        void ActNothing(const Event1 &e)
        {
            // nothing 
        }

        void SwitchToS3(const Event1 &e)
        {
            m_fsm.SetState(st3);
        }

        void SwitchToS4(const Event1 &e)
        {
            m_fsm.SetState(st4);
        }

        void InitTable(Fsm &fsm)
        {
            UTILS_FSM_ADD_EXT( st1, Event1, ActNothing, (st2)           );
            UTILS_FSM_ADD_EXT( st2, Event1, SwitchToS3, (st1, st2, st3) );
            UTILS_FSM_ADD_EXT( st3, Event1, SwitchToS4, (st1, st2, st3) );  // runtime error
            UTILS_FSM_ADD_EXT( st5, Event1, ActNothing, (_DynamicState_) ); 

            // test for dublicates in end states
            {
                ESS::ExceptionHookRethrow<> hook;

                bool wasException = false;
                try
                {
                    UTILS_FSM_ADD_EXT( st4, Event1, ActNothing, (st1, st2, st1) );  
                }
                catch(ESS::HookRethrow &e)
                {
                    wasException = true;
                }
                TUT_ASSERT(wasException); 
            }
        }

        void Run()
        {
            Event1 e;

            m_fsm.DoEvent(e);  // to s2
            m_fsm.DoEvent(e);  // to s3

            // verify assert for exit in non-declared state 
            {
                ESS::ExceptionHookRethrow<> hook;

                bool wasException = false;
                try
                {
                    m_fsm.DoEvent(e);  // to s4
                }
                catch(ESS::HookRethrow &e)
                {
                    wasException = true;
                }
                TUT_ASSERT(wasException); 
            }

        }

    public:

        Test() : m_fsm(st1)
        {
            InitTable(m_fsm);
            Run();
        }
    };


}  // namespace X4


// --------------------------------------

namespace UtilsTests
{
    void NewFsmTest()
    {
        X1::Game game1;
        X2::OnlineGame game2;
        X3::Test test;
        X4::Test test2;
    }

} // namespace UtilsTests


