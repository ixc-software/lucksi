#ifndef ISDNTEST_H
#define ISDNTEST_H

#include "Domain/DomainClass.h"
#include "iCore/MsgObject.h"
#include "iCore/MsgThread.h"
#include "iCore/MsgTimer.h"
#include "Utils/Random.h"
#include "ISDN/L2Profiles.h"
#include "TestFsm.h"
#include "infratest.h"
#include "DrvLinkLayer.h"
#include "DriverL1Test.h"
#include "ISDN/isdnl2.h"
#include "l3test.h"
#include "LogOut.h"
#include "iLog/LogSessionProfileSetup.h"
#include "ITestResult.h"

namespace IsdnTest
{        	
    using Domain::DomainClass;
    using ISDN::L2Profile;
	
	class LapdTest;
    ITestState* GetStartTest(LapdTest&); // definition in StateImpl.cpp

    class NObjLapdTestSettings;

	// Тип-тест Lapd - процедуры. Адаптирован для запуска из под TestCollector
    class LapdTest 
        : boost::noncopyable,
        public iCore::MsgObject        
	{            
	public:        
        LapdTest(DomainClass &domain, const NObjLapdTestSettings &profile, ITestResult& callback);        
    
    public:

        ~LapdTest();                      
        void SetComplete();
        void AbortTest();
        void StopTest();
		ISDN::IsdnL2& GetL2Left() { return m_L2Left;  }
		ISDN::IsdnL2& GetL2Right(){ return m_L2Right; }
		L3Test& GetL3Left() { return m_L3Left;  }
		L3Test& GetL3Right(){ return m_L3Right; }
        DriverL1Test& GetDrvLeft()   { return m_drvLeft; }
        DriverL1Test& GetDrvRight()  { return m_drvRight; }
        Logger* GetLogger() { return &m_logSession; }
        const NObjLapdTestSettings& GetProfile() const {return m_prof;}        
        typedef NObjLapdTestSettings TNObjProfile;               
        TestFsm& getFsm() {return m_fsm;}    

	private: 
        void StartTest();    
        void DriLog(QString msg);
        void testFailed(iCore::MsgTimer *pT);

        ITestResult& m_callback;
        TestFsm m_fsm;
        const NObjLapdTestSettings& m_prof;
        ISDN::LeftDrvRole m_LDR;
        ISDN::RightDrvRole m_RDR;
        Utils::Random m_rnd;
        
        iLogW::ILogSessionToLogStore& m_logStore;                

		IsdnInfraTest m_infrLeft, m_infrRight;
		DriverL1Test m_drvLeft, m_drvRight;
        ISDN::IsdnL2 m_L2Left, m_L2Right;
		L3Test m_L3Left, m_L3Right;
		Logger m_logSession;        
		DrvLinkLayer m_p2pLeft, m_p2pRight;
        Utils::AtomicBool m_complete;  
        iCore::MsgTimer m_t;
        dword m_maxDuration;
        iLogW::LogRecordTag m_tagLeftStat;
        iLogW::LogRecordTag m_tagRightStat;        
	};        

}//IsdnTest

#endif
