#ifndef TESTFSM_H
#define TESTFSM_H

// #include "IsdnTestConfig.h"
#include "ITestState.h"
#include "ITestFsm.h"
#include "logger.h"

#include "Platform/Platform.h"
#include "Utils/ErrorsSubsystem.h"


#include "iCore/MsgObject.h"
#include "iCore/MsgThread.h"
#include "iCore/MsgTimer.h"

namespace IsdnTest
{

	
	class TestFsm: public ITestFsm, public iCore::MsgObject
	{
	public:
        TestFsm(iCore::MsgThread& thread):
          iCore::MsgObject(thread),
          m_tagChangeState(iLogW::LogRecordTag()),
		  m_pCurrState( 0 ),
		  m_pLogger(0),
          m_t(this, &TestFsm::Process) {}
		
		~TestFsm() { delete m_pCurrState; }
		
		void SetStartTest(ITestState* start) // override
		{
            m_pCurrState = start;
        }		
		
		virtual void SwitchTo(ITestState* pNewState) // override,virtual
		{
            TUT_ASSERT(m_pLogger);            
			Writer(*m_pLogger, m_tagChangeState).Write() << "From " << m_pCurrState->GetName();
			m_pCurrState = pNewState;
			Writer(*m_pLogger, m_tagChangeState).Write() << "To " << pNewState->GetName();            
		}
		
		dword GetTick() // override
		{
            return Platform::GetSystemTickCount();
        } 
    
    

        void Start(int period) { m_t.Start(period,true); }
        void Stop() {m_t.Stop();}
        void SetLogger(Logger *pLogger) 
        {
            m_pLogger = pLogger;
            m_tagChangeState = pLogger->RegNewTag("Change State");
        }

	private:

        iLogW::LogRecordTag m_tagChangeState;

		ITestState* m_pCurrState;
		Logger *m_pLogger; // set from IsdnTestFsm
        iCore::MsgTimer m_t;

        void Process(iCore::MsgTimer *pT)
		{
            TUT_ASSERT(m_pCurrState);
			m_pCurrState->Process(); 
		}

	};

}//namespace IsdnTest

#endif
