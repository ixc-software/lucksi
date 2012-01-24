#include "stdafx.h"
#include "LapdTestClass.h"
#include "NObjLapdTestSettings.h"
#include "iLog/LogWrapperLibrary.h"
#include "iLog/LogManager.h"

namespace IsdnTest
{

    using boost::shared_ptr;
    
    LapdTest::LapdTest(DomainClass &domain, const NObjLapdTestSettings &profile, ITestResult& callback)
        : iCore::MsgObject(domain.getMsgThread()),
        m_callback(callback),
        m_prof(profile),
        m_fsm( domain.getDomain().getMsgThread() ),     
        m_logStore(domain.getDomain().Log().getSessionInterface()),                            
        m_infrLeft(&m_rnd, m_logStore, domain.getDomain().Log().getLogSessionProfile()),
        m_infrRight(&m_rnd, m_logStore, domain.getDomain().Log().getLogSessionProfile()),

        m_drvLeft(domain.getDomain(), &m_infrLeft, m_LDR, m_prof), 
        m_drvRight(domain.getDomain(), &m_infrRight, m_RDR, m_prof),                       

        m_L2Left(domain.getDomain(), m_drvLeft.GetBinder(), &m_infrLeft, L2Profile::CreateAsTE(*m_prof.m_pL2TraceOption) ),
        m_L2Right(domain.getDomain(), m_drvRight.GetBinder(), &m_infrRight, L2Profile::CreateAsNT(*m_prof.m_pL2TraceOption) ),

        m_L3Left( domain.getDomain() ),
        m_L3Right( domain.getDomain() ),

        // todo: создавать сессию
        m_logSession("MainTestFsm", m_logStore, m_prof.m_TracedTest, domain.Log().getLogSessionProfile()),

        m_p2pLeft(&m_drvLeft, domain.getDomain().getMsgThread() ),
        m_p2pRight(&m_drvRight, domain.getDomain().getMsgThread() ),
        m_complete(false),          
        m_t(this, &LapdTest::testFailed), 
        m_maxDuration(m_prof.m_MaxDurationMsec),
        m_tagLeftStat(m_logSession.RegNewTag("Left L2StatData")),
        m_tagRightStat(m_logSession.RegNewTag("Right L2StatData"))        
    {
        m_t.Start(m_maxDuration);
        m_fsm.SetLogger(&m_logSession);

        //сборка				
        m_L2Left.SetUpIntf(&m_L3Left, &m_L3Left);
        m_L2Right.SetUpIntf(&m_L3Right, &m_L3Right);

        m_L3Left.SetDownIntf(&m_L2Left);
        m_L3Right.SetDownIntf(&m_L2Right);
        m_fsm.SetStartTest( GetStartTest(*this) );

        m_p2pLeft.SetOpposite(&m_p2pRight);
        m_p2pRight.SetOpposite(&m_p2pLeft);

        m_drvLeft.SetDownIntf(&m_p2pLeft);
        m_drvRight.SetDownIntf(&m_p2pRight);

        StartTest();
    }

    LapdTest::~LapdTest()
    {
        StopTest();  

        ISDN::L2StatData l2statRight = m_L2Right.getStatData();
        m_logSession.Log(l2statRight.getAsString(), m_tagLeftStat);

        ISDN::L2StatData l2statLeft = m_L2Left.getStatData();
        m_logSession.Log(l2statLeft.getAsString(), m_tagLeftStat);
    }

    void LapdTest::StartTest()
    {        
        m_L2Left.StartProcessing(50);
        m_L2Right.StartProcessing(50);
        m_fsm.Start(50); // run TestFsm
        m_t.Start(m_maxDuration);
    }

    void LapdTest::SetComplete()
    {
        StopTest();          
        m_callback.TestFinished(true, "TestComplete");
    }

    void LapdTest::StopTest()
    {
        m_fsm.Stop();
        m_t.Stop();
        m_L2Left.StopProcessing();
        m_L2Right.StopProcessing();
    }  

    void LapdTest::testFailed( iCore::MsgTimer *pT )
    {
        StopTest();
        m_callback.TestFinished(false, "General Timeout error in IsdnTest");        
    }

    void LapdTest::AbortTest()
    {        
        StopTest();                
    }

    
} // namespace IsdnTest

