#include "stdafx.h"
#include "Platform/Platform.h"
#include "iCore/MsgObject.h"
#include "iCore/ThreadRunner.h"
#include "iCore/MsgTimer.h"
#include "BfDev/AdiTimerTest.h"
#include "AdiTimerTestRun.h"



namespace 
{
	using namespace BfDev;		
	using namespace Platform;	
	
	struct AppParams {};
	
	const int CProcessTimeoutMs = 5;
	const int CTestTimeoutMs = 20000;

    class Application : boost::noncopyable,
        public iCore::MsgObject
        
    {
    	typedef Application T;
    public:
        Application(iCore::IThreadRunner &runner, AppParams &param) :
            iCore::MsgObject(runner.getThread()),
            m_runner(runner),
			m_timer(this, &T::OnProcess),
			m_adiTimerTest(1, 2)
        {
        	m_timer.Start(CProcessTimeoutMs);
        	m_timerCount = 0;
        	m_startTick = Platform::GetSystemTickCount();
        	m_adiTimerTest.Start(2);
       
        }
    private:                
	    void OnProcess( iCore::MsgTimer* )
	    {
	    	++m_timerCount;
        	if(Platform::GetSystemTickCount() - m_startTick > CTestTimeoutMs)
        		EndTest();
			else	    	
	    		m_timer.Start(CProcessTimeoutMs);
	    }
	    	
	    void EndTest()
	    {
	    	m_adiTimerTest.Stop();
	    	
	    	std::cout << "Timer count: " << m_timerCount << std::endl
	    		<< "Result:" << std::endl
	    		<< m_adiTimerTest.Result(CTestTimeoutMs * 1000) << std::endl;
	    	m_runner.getCompletedFlag().Set(true);        	    
	    }
    private:        
    	iCore::IThreadRunner &m_runner;
    	iCore::MsgTimer m_timer;
    	AdiTimerTest m_adiTimerTest;
    	dword m_timerCount;
    	dword m_startTick;
    };
};

namespace AdiTest
{
	void Run()
	{
		AdiTimerTest test(1, 2);
		BfDev::BfTimerCounter m_sleepTimer(3);
		test.Start(2);
		dword count = BfTimerHelper::NsTimerToCounter(2000000000);
		m_sleepTimer.Sleep(count);
   		test.Stop();
        std::cout << test.Result() << std::endl;		        
	}
	
	
	void RunThreadTest()
	{
		Run();
	    AppParams params;
	    // run
	    iCore::ThreadRunner test(Platform::Thread::NormalPriority, 0);
	    bool res = test.Run<Application>(params);

	} 

};


