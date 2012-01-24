
#include "stdafx.h"
#include "fsm.h"
#include "Utils/ErrorsSubsystem.h"


namespace ISDN
{

	FsmMachine::FsmMachine(FsmNode *fnlist, int fncount,
			/*Fsm()*/int stateCount,int eventCount, Fsm::EventArr* Event, Fsm::StateArr* State, //FsmInit
			IIsdnClock* clock, ILoggable* pLog, //SetIntf
			IsdnL2* owner,int userint, const std::string& fsmName) : //SetInstance
                    m_logFsmTag(pLog->RegNewTag(fsmName)),
                    m_logTimerTag(pLog->RegNewTag(fsmName + " timer")),
					m_Fsm(stateCount, eventCount, Event, State),
					m_clock(clock),
					m_pLog(pLog)
                    //m_fsmName(fsmName)
	{
		FsmInit(fnlist, fncount);
		SetInstace(owner, userint);
		m_state = -1;
	}

	inline dword FsmMachine::CalcIndex(FsmNode *fnlist,int i)
	{
		return m_Fsm.state_count * fnlist[i].event + fnlist[i].state;
	}


	void FsmMachine::FsmInit(FsmNode *fnlist, int fncount)							
	{
		for (int i = 0; i < fncount; i++) 
		{
            ESS_ASSERT(fnlist[i].state < m_Fsm.state_count);
            ESS_ASSERT(fnlist[i].event < m_Fsm.event_count);
            ESS_ASSERT(fnlist[i].routine);

            dword indx = CalcIndex(fnlist, i);
			m_Fsm.jumpvector.at(indx) = fnlist[i].routine;
		}
	}


	int FsmMachine::Event(int event, void *arg)
	{
		FSMFNPTR r;

        ESS_ASSERT(m_state < m_Fsm.state_count);
        ESS_ASSERT(event < m_Fsm.event_count);
        ESS_ASSERT(m_state >= 0);
        ESS_ASSERT(event >= 0);

        /*if (m_state == 5 && event == 11)
            ESS_ASSERT("Break");*/

		r = m_Fsm.jumpvector.at(m_Fsm.state_count * event + m_state);

		if (r) 
        {
			if (m_pLog->getTraceOn())
			{                
				LogWriter(m_pLog, m_logFsmTag).Write() << "State " << m_Fsm.strState[m_state]
                << " Event " << m_Fsm.strEvent[event];
			}
			
			r(this, event, arg);
			return (0);
		} 

		if (m_pLog->getTraceOn())
		{
			LogWriter(m_pLog, m_logFsmTag).Write() << "State " << m_Fsm.strState[m_state]
				<< " Event " << m_Fsm.strEvent[event] << " no routine.";
		}

		return (!0);    
	}

	void FsmMachine::ChangeState(int newstate)
	{		
        ESS_ASSERT(( (newstate<m_Fsm.state_count) && (newstate>=0) ) );
		m_state = newstate;
		if (m_pLog->getTraceOn())
		{
			LogWriter(m_pLog, m_logFsmTag).Write() << "ChangeState to "
                << m_Fsm.strState[newstate];
		}		
	}


	int FsmMachine::AddTimer(FsmTimer *t)
	{
		if( t->IsInList() )
		{
			if (m_pLog->getTraceOn())
				LogWriter(m_pLog, m_logTimerTag).Write() 
                << "Warning: timer " << m_Fsm.strEvent[t->m_event] << " already added! ";
			return -1;
		}
		
		m_timers.push_back(t);
		return 0;
	}

	void FsmMachine::RemoveTimer(FsmTimer *t)//del timer
	{
		if( !t->IsInList() )
		{
			if (m_pLog->getTraceOn())
				LogWriter(m_pLog, m_logTimerTag).Write()
                    << "Warning:  try to remove deactivated timer";
			return ;
		}
		m_timers.remove(t); // !не удаляет объект, только из списка
		t->ResetInList();
	}

	void FsmMachine::Process()
	{
		//unsigned long currT;// = clock->GetTic();
		FsmTimer* pTimer;
		
        //while(!m_timers.empty())                  // - обрабатывать пока не сработают все таймера
		for(int i = m_timers.size(); i > 0 ;--i)    // - один проход по всем таймерам. Несработавшие остаются в очереди
        {
			pTimer=m_timers.front();
			m_timers.pop_front();
			if( !pTimer->Process(m_clock->GetTick()) )
				m_timers.push_back(pTimer);
			//else pTimer->ResetInList();
		}
        
	}

	void FsmMachine::SetInstace(IsdnL2* owner,int userint)
	{
		m_pOwner=owner;
		m_userint=userint;
	}

	int FsmTimer::Set(int millisec, int event, void *arg)
	{
		dword curr_t = m_pOwnMachine->m_clock->GetTick();
		m_expTime = millisec +  curr_t; // или метод или свои часы иметь надо
		m_shift = dword(0) - m_expTime;
		m_event=event;
		m_arg=arg;
		int ret = m_pOwnMachine->AddTimer(this);
        if (m_pOwnMachine->m_pLog->getTraceOn() && !ret)
		{
			LogWriter(m_pOwnMachine->m_pLog, m_pOwnMachine->m_logTimerTag).Write() << "-Set timer " 
                << m_pOwnMachine->m_Fsm.strEvent[event] << " period = " << millisec;
		}
		if (!ret) m_inList = true;
		return ret;
	}

	void FsmTimer::Restart(int millisec, int event, void *arg)
	{
        if (m_pOwnMachine->m_pLog->getTraceOn())
		{
			LogWriter(m_pOwnMachine->m_pLog, m_pOwnMachine->m_logTimerTag).Write() << "-Restart timer " 
                << m_pOwnMachine->m_Fsm.strEvent[event] << " period = " << millisec;
		}
		if(m_inList)m_pOwnMachine->RemoveTimer(this);
		Set(millisec, event, arg);	
	}

	bool FsmTimer::Process(dword ticks)
	{
        ESS_ASSERT(m_inList && "Call process without Set or timer not active");
		if (ticks + m_shift >= Half) return false;
        ResetInList();
		m_pOwnMachine->Event(m_event, m_arg);	   
		return true;
	}


}   // namepsace ISDN 

