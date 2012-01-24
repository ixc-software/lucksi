#pragma once

#include "IBasicInterface.h"
#include "ErrorsSubsystem.h"
#include "ManagedList.h"
#include "ComparableT.h"
#include "FullEnumResolver.h"
#include "IntToString.h"
#include "ContUnique.h"
#include "FuncRAII.h"

namespace Utils
{
    struct FsmSpecState
    {
        enum Enum
        {
            // for begin state
            _AnyState_,

            // for end state 
            _DynamicState_,
            _SameState_,
        };

        static bool VerifyFsmEnum(Enum val)
        {
            return (val == _AnyState_ || val == _DynamicState_ || val == _SameState_);
        }

        static std::string FsmEnumToString(Enum v)
        {
            if (v == _AnyState_)        return "_AnyState_";
            if (v == _DynamicState_)    return "_DynamicState_";
            if (v == _SameState_)       return "_SameState_";

            ESS_HALT( Utils::IntToString(v) );
            return "(none)";
        }
    };

    // ------------------------------------------------------------

    class FsmTable 
    {
        int m_index;

    public:
        FsmTable(int index = -1) : m_index(index) { }

        int Index() const 
        { 
            ESS_ASSERT(m_index >= 0);
            return m_index; 
        }

    };

    // ------------------------------------------------------------

    /*
        Classic mode  -- only _AnyState_ allowed, search for best matching thru all table 

        Extended mode -- _AnyState_ and *any event* allowed, processing search 
                             till first record which can process event; order of records is matter 

        Differences vs Utils::StateMachine
        - table will be created in runtime, not type list at compile time
        - polymorphic (thru typeid) events route, not thru events type
        - machine enum state is type safe 
        - extended mode (*any event* support, record's order is matter)
        - in recursive mode, for concrete end state record, there is no check for 
          state after handle called 

        (*) -- to verify result of boost::bind it should be explicit converted to 
                   boost::function<void (const TEvent&)>            or
                   boost::function<void (const TDerivedEvent&)>
                   before passed to Dir()
    */

    template<class TEvent, class TStateEnum>
    class Fsm : boost::noncopyable
    {
        BOOST_STATIC_ASSERT( boost::is_enum<TStateEnum>::value );
        BOOST_STATIC_ASSERT( boost::has_virtual_destructor<TEvent>::value );

        // invariant vector<TStateEnum> or FsmSpecState::Enum
        class State : public ComparableT<State>
        {
            bool m_isSpec;

            FsmSpecState::Enum m_specState;
            std::vector<TStateEnum> m_fsmStates;

        public:

            State(FsmSpecState::Enum val) : m_isSpec(true), m_specState(val)
            {
                ESS_ASSERT( FsmSpecState::VerifyFsmEnum(val) );
            }

            State(TStateEnum val) : m_isSpec(false)
            {
                m_fsmStates.push_back(val);
            }

            State(const std::vector<TStateEnum> &v) : m_isSpec(false), m_fsmStates(v)
            {
                ESS_ASSERT(v.size() > 0);
                ESS_ASSERT( AllItemsUnique(v) );
            }

            bool IsSpec() const { return m_isSpec; }

            FsmSpecState::Enum SpecState() const 
            { 
                ESS_ASSERT(m_isSpec);
                return m_specState; 
            }

            TStateEnum FsmState() const 
            { 
                ESS_ASSERT( FsmStateSingle() );

                return m_fsmStates.at(0); 
            }

            bool FsmStateSingle() const
            {
                return ( m_fsmStates.size() == 1 );
            }

            bool StateIn(TStateEnum e) const
            {
                ESS_ASSERT(m_fsmStates.size() > 0);

                return 
                    std::find(m_fsmStates.begin(), m_fsmStates.end(), e) != m_fsmStates.end();
            }

            
            int Compare(const State &other) const
            {
                if (m_isSpec != other.m_isSpec)
                {
                    return CompareToInt(m_isSpec, other.m_isSpec);
                }

                return m_isSpec ? 
                    CompareToInt(m_specState, other.m_specState) :
                    CompareContToInt(m_fsmStates, other.m_fsmStates);
            }

            // only for _DynamicState_ or multistate 
            bool CanSwitchTo(TStateEnum newState) const
            {
                if ( IsSpec() )
                {
                    return (SpecState() == FsmSpecState::_DynamicState_); 
                }

                return !FsmStateSingle() && StateIn( newState );
            }

            std::string ToString() const
            {
                if ( IsSpec() )
                {
                    return FsmSpecState::FsmEnumToString( SpecState() );
                }

                return Utils::EnumResolve( FsmState() );
            }

        };

        class IDir : public Utils::IBasicInterface
        {
        public:
            virtual const State& Begin() const = 0;
            virtual const State& End()   const = 0;
            virtual const std::string& Event() const = 0;
            virtual bool AnyEvent() const = 0;
            virtual void Exec(const TEvent &e) const = 0;
        };

        template<class TDirEvent, class TFn>
        class Dir : public IDir
        {
            const std::string m_typeName;
            const State m_begin, m_end;
            const TFn m_fn;

        // IDir impl
        private:

            const State& Begin() const { return m_begin; }
            const State& End()   const { return m_end; }
            const std::string& Event() const { return m_typeName; }

            bool AnyEvent() const
            {
                return boost::is_same<TEvent, TDirEvent>::value;
            }

            void Exec(const TEvent &e) const 
            {
                m_fn( static_cast<const TDirEvent&>(e) );
            }

        public:

            Dir(State begin, State end, const TFn &fn) : 
              m_typeName( typeid(TDirEvent).name() ),
              m_begin(begin), m_end(end), m_fn(fn)
            {
                // ESS_ASSERT(fn);  // result of boost::bind can't be verified as bool in runtime (see *)

                if ( begin.IsSpec() ) 
                {
                    ESS_ASSERT( begin.SpecState() == FsmSpecState::_AnyState_ );
                }
                else
                {
                    ESS_ASSERT( begin.FsmStateSingle() ) ;  // verify -- begin has only one fsm value
                }

                if ( end.IsSpec() )
                {
                    ESS_ASSERT( end.SpecState() != FsmSpecState::_AnyState_ );                    
                }
            }

        };

        typedef boost::function<void (const TEvent&)> NoTransitionFn;
        typedef boost::function<void (TStateEnum prevState, TStateEnum newState)> ChangeStateFn;
        typedef Utils::ManagedList<IDir> Table;

        TStateEnum m_state;
        const bool m_extMode;
        bool m_closed;
        ChangeStateFn m_changeFn;
        NoTransitionFn m_noTransFn;

        Utils::ManagedList<Table> m_tables;

        // recursive/non-recursive mode
        bool m_recursive;
        boost::scoped_ptr<State> m_callEndState;

        enum CmpResult
        {
            // bad
            crBadDir, 

            // OK, exact matching; this list is ordered!
            crNone,
            crState,
            crEvent,
            crBoth,
        };

        static CmpResult Compare(TStateEnum state, const std::string &evType, const IDir &d)
        {
            // state 
            bool stateMatch = false;            

            if ( d.Begin().IsSpec() )
            {
                ESS_ASSERT(d.Begin().SpecState() == FsmSpecState::_AnyState_);
                // no exact match
            }
            else
            {
                if (d.Begin().FsmState() != state) return crBadDir;
                stateMatch = true;
            }

            // event
            bool eventMatch = false;

            if ( d.AnyEvent() )
            {
                // no exact match
            }
            else
            {
                if ( evType != d.Event() ) return crBadDir;
                eventMatch = true;
            }

            // result
            if (stateMatch && eventMatch) return crBoth;
            if (stateMatch) return crState;
            if (eventMatch) return crEvent;

            return crNone;
        }

        IDir* ExtDirFind(Table &t, const std::string &evType)
        {
            for(int i = 0; i < t.Size(); ++i)
            {
                CmpResult r = Compare(m_state, evType, *t[i]);
                if (r != crBadDir) return t[i];
            }

            return 0;
        }

        IDir* ClassicDirFind(Table &t, const std::string &evType)
        {
            CmpResult best = crBadDir;
            IDir *pDir = 0;

            for(int i = 0; i < t.Size(); ++i)
            {
                CmpResult r = Compare(m_state, evType, *t[i]);
                if (r > best) 
                {
                    best = r;
                    pDir = t[i];
                    if (best == crBoth) break;
                }
            }

            return pDir;
        }

        IDir* FindEqualDirection(Table &t, IDir *p)
        {
            for(int i = 0; i < t.Size(); ++i)
            {
                if (p->Event() == t[i]->Event() && 
                    p->Begin() == t[i]->Begin())
                {
                    return t[i];
                }
            }

            return 0;
        }

        static State EndState(TStateEnum *ps0, TStateEnum *ps1 = 0, TStateEnum *ps2 = 0, 
            TStateEnum *ps3 = 0, TStateEnum *ps4 = 0)
        {
            std::vector<TStateEnum> v;
            v.push_back(*ps0);

            if (ps1)
            {
                v.push_back(*ps1);
                if (ps2)
                {
                    v.push_back(*ps2);
                    if (ps3)
                    {
                        v.push_back(*ps3);
                        if (ps4)
                        {
                            v.push_back(*ps4);
                        }
                    }
                }
            }

            return State(v);
        }

    public:

        Fsm(TStateEnum initialState, bool extMode = false, 
            NoTransitionFn noTransFn = NoTransitionFn(),
            ChangeStateFn changeFn = ChangeStateFn()) : 
          m_state(initialState), m_extMode(extMode), m_closed(false),
          m_changeFn(changeFn), m_noTransFn(noTransFn),
          m_recursive(false)
        {
        }

        void EnableRecursiveMode()  // temporary fix for HwFinder
        {
            m_recursive = true;
        }

        template<class TEvType, class TStBegin, class TStEnd, class TFn>
        void Add(TStBegin stBegin, TStEnd stEnd, const TFn &fn)
        {
            const bool CEvTypeCheck = boost::is_base_of<TEvent, TEvType>::value;
            BOOST_STATIC_ASSERT( CEvTypeCheck );

            ESS_ASSERT(!m_closed);

            // last table
            if ( m_tables.IsEmpty() ) m_tables.Add( new Table() );
            Table &t = *(m_tables[m_tables.Size() - 1]);

            // check -- any event allows only in extended mode
            const bool CEvAny = boost::is_same<TEvent, TEvType>::value;
            if (CEvAny) ESS_ASSERT(m_extMode);

            // create new record
            IDir *p = new Dir<TEvType, TFn>( State(stBegin), State(stEnd), fn );
            bool dublicate = (FindEqualDirection(t, p) != 0);

            t.Add(p);

            // after Add() for no memory leak :)
            if (dublicate)
            {
                std::string s = "Dublicate [ev, state] as [" + 
                    Platform::FormatTypeidName( typeid(TEvType).name() ) + ", " + 
                    p->Begin().ToString() + "]";

                ESS_HALT(s);
            }            
        }

        // close current table, return handle to it, Add() will add record to new table
        FsmTable CloseTable()
        {
            ESS_ASSERT(m_tables.Size() > 0); 
            m_tables.Add( new Table() );
            return m_tables.Size() - 2;  // index of last item before Add() new table
        }

        void DoEvent(const TEvent &e, const FsmTable &tableIndex = FsmTable(0))
        {
            if (!m_recursive && m_callEndState) 
            {
                ESS_HALT("FSM recursive!");
            }

            if (!m_closed)
            {
                ESS_ASSERT(m_tables.Size() > 0);
                m_closed = true;  // lock table 
            }
            
            std::string evType = typeid(e).name();

            // assert -- not TEvent (not base class)
            ESS_ASSERT( evType != typeid(TEvent).name() );

            Table &t = *(m_tables[tableIndex.Index()]);
            ESS_ASSERT( !t.IsEmpty() );

            IDir *pDir = (m_extMode) ? ExtDirFind(t, evType) : ClassicDirFind(t, evType);

            if (pDir == 0)
            {
                if (m_noTransFn) m_noTransFn(e);
                return;
            }

            TStateEnum prevState = GetState();

            {
                if (!m_recursive) m_callEndState.reset( new State( pDir->End() ) );
                
                FuncRAII f( FuncRAII::PtrReset(m_callEndState) ); 
               
                pDir->Exec(e);                    
            }

            // verify + switch
            {
                const State &e = pDir->End();

                if ( e.IsSpec() )
                {
                    if (e.SpecState() == FsmSpecState::_DynamicState_); // nothing

                    if (e.SpecState() == FsmSpecState::_SameState_) // state can't change
                    {
                        ESS_ASSERT(GetState() == prevState);   
                    }
                }
                else
                {
                    if ( e.FsmStateSingle() )
                    {
                        if (!m_recursive) ESS_ASSERT(GetState() == prevState);      // state can't change (for non-recursive only)  
                        SetState( e.FsmState() );                                   // switch to new state
                    }
                    else
                    {
                        ESS_ASSERT( e.StateIn( GetState() ) );
                    }
                }
            }
        }

        void SetState(TStateEnum newState)
        {
            if (!m_recursive && m_callEndState)
            {
                ESS_ASSERT( m_callEndState->CanSwitchTo(newState) );
            }

            if (m_state == newState) return;

            // set
            TStateEnum prevState = m_state;
            m_state = newState;

            // notify
            if (m_changeFn) m_changeFn(prevState, newState);
        }

        TStateEnum GetState() const
        {
            return m_state;
        }

        // service
        static State EndState(FsmSpecState::Enum e) { return State(e); }
        static State EndState(TStateEnum s0) { return EndState(&s0); }
        static State EndState(TStateEnum s0, TStateEnum s1) { return EndState(&s0, &s1); }
        static State EndState(TStateEnum s0, TStateEnum s1, TStateEnum s2) { return EndState(&s0, &s1, &s2); }
        static State EndState(TStateEnum s0, TStateEnum s1, TStateEnum s2, TStateEnum s3) 
                                    { return EndState(&s0, &s1, &s2, &s3); }
        static State EndState(TStateEnum s0, TStateEnum s1, TStateEnum s2, TStateEnum s3, TStateEnum s4) 
                                    { return EndState(&s0, &s1, &s2, &s3, &s4); }        

    };  

    // -----------------------------------------------------------------

    /* 
       Macro is waiting: fsm as Utils::Fsm, T as method owner  

       e.g.: fsm.Add<GamerRecover>( Dead, Alive, boost::bind(&T::RiseFromDead, this, _1) );
    */

    #define UTILS_FSM_ADD(begin, ev, method, end) fsm.Add<ev>(begin, end, boost::bind(&T::method, this, _1) )


    /*
        Version with end states vector support
    */

    #define UTILS_FSM_ADD_EXT(begin, ev, method, end) fsm.Add<ev>(begin, fsm.EndState##end, boost::bind(&T::method, this, _1) )


    /*
        Non macro solution for Add() can be something like that 

        ...  // inside fsm holder method
        Fsm::Adder a = m_fsm.Adder(this);  // lock this + fsm in adder
        a.Add<Event>(stBegin, stEnd, &T::Handler);

    */
    
    
}  // namespace Utils

