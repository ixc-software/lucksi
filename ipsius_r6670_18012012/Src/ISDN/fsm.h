/* 

 */

#ifndef __FSM_H__
#define __FSM_H__

#include "Utils/ErrorsSubsystem.h"
#include "IIsdnLog.h"
#include "IIsdnClock.h"
#include "Platform/PlatformTypes.h"



namespace ISDN
{

    //------------------------------------------------------------------------------------
	class FsmMachine;
	typedef void (* FSMFNPTR)(FsmMachine *, int, void *);
	
	struct Fsm {
	typedef const char* EventArr;
	typedef const char* StateArr;

	Fsm(int stateCount, int eventCount, EventArr* Event, StateArr* State): //__init IsdnXXNew
		jumpvector( stateCount*eventCount )
	{
		state_count = stateCount;
		event_count = eventCount;
		strEvent = Event;
		strState = State;
	}
	std::vector<FSMFNPTR> jumpvector;
	int state_count, event_count;
	EventArr *strEvent;
	StateArr *strState;
	};
    //------------------------------------------------------------------------------------

	struct FsmNode {
		int state, event;
		FSMFNPTR routine;
	};

    //------------------------------------------------------------------------------------
	class IsdnL2;   //fw-decl for userdata
	class FsmTimer;

	class FsmMachine
	{
		friend class FsmTimer;

	public:
		
        FsmMachine(FsmNode *fnlist, int fncount,
			/*Fsm()*/int stateCount, int eventCount, Fsm::EventArr* Event, Fsm::StateArr* State, //FsmInit
			IIsdnClock* clock, ILoggable* pLog, //SetIntf
            IsdnL2* owner, int userint, const std::string& fsmName);//SetInstance
			
		~FsmMachine()
		{/*delete []m_Fsm.jumpmatrix*/;/*free ptr in timer list?*/}
		//---------------------------------------------------------------------
		int Event(int event, void *arg);
		
        void ChangeState(int newstate);//void FsmChangeState
		
        int GetState()
		{
			//if(m_state=-1)throw Except("Machine start state not be installed ");
            ESS_ASSERT( (m_state!=-1) && "Machine start state not be installed");
			return m_state;
		}
		
        int AddTimer(FsmTimer *t);
		
        void Process();
		
        IsdnL2* GetUserData(){return m_pOwner;}
	
    private:
		
        void FsmInit(FsmNode *fnlist, int fncount);		
		
        void SetInstace(IsdnL2* owner,int userint);
		
        dword CalcIndex(FsmNode *fnlist,int i);
		
        void RemoveTimer(FsmTimer *t); // del timer
	    
        //data fields
        iLogW::LogRecordTag m_logFsmTag;
        iLogW::LogRecordTag m_logTimerTag;
		std::list<FsmTimer*> m_timers;		
		Fsm m_Fsm;
		int m_state;		
		IsdnL2 *m_pOwner;        
		int m_userint;
		IIsdnClock *m_clock;        
		ILoggable* m_pLog;
        //std::string m_fsmName;
			
	};
//----------------------------------------------------------------------------------


	class FsmTimer
	{
		friend class FsmMachine;

	public:
		
        FsmTimer(FsmMachine* fsm):m_pOwnMachine(fsm),m_inList(false),
			m_event(-1),
			m_arg(0),
			m_shift(0)  {}
		
		//Set and activate timer
		int Set(int millisec, int event, void *arg);
		
        void Restart(int millisec, int event, void *arg);
		
        void Deactivate(){ m_pOwnMachine->RemoveTimer(this); }
		
	private:
		
        dword GetExp(){return m_expTime;}
		
        bool IsInList()const{return m_inList;}//timer_pending
		
        bool Process(dword ticks);
		
        void SetInList(){m_inList=true;}
		
        void ResetInList(){m_inList=false;}
		
        //data fields:		
        FsmMachine * m_pOwnMachine;
		bool m_inList;
		dword m_expTime;
		int m_event;
		void* m_arg;
		dword m_shift;
		static const dword Half = ~dword(0)/2;

	};

}// ISDN::

#endif
