#include "stdafx.h"
#include "ISDN/BDistributionStrategies.h"
#include "DssScenario.h"
#include "Utils/ExeName.h"
#include "iLog/LogWrapperLibrary.h"
#include "iLog/LogManager.h"

namespace IsdnTest
{

    using Domain::DomainClass;

    using Utils::SafeRef;
    using ISDN::IDssCall;
    using ISDN::DssCallParams;
    using boost::shared_ptr;
    using ISDN::DssWarning; 
    using ISDN::L3Profile;
    using ISDN::L2Profile;

    class DssScenario::Fsm
        : public IScnFsm
    {
        //static const int CMaxTryCount = 0;

        enum State
        {
            st_null,
            //st_DssDeactive,
            st_waitActivation,
            st_waitDeactivation,
            //st_DssActive
        };

        State m_state;
        bool m_userSideIsActive;
        bool m_netSideIsActive;
        //int m_tryCount;

        IsdnStack* m_waitRestartCompleteFrom;
        Utils::IVirtualInvoke *m_restartCompleteAction;

        DssScenario* m_pOwner;        

        bool InState( State state, const std::string& eventName )
        {
            bool ret = state == m_state;
            if (!ret)
            {                
                m_pOwner->m_logSession.Log( "Unexpected command " + eventName/*kind unexp...*/ );
            }
            return ret;
        }

    // Implementation IScnFsm:
    private:

        void Activated(IsdnStack* from) 
        {
            if ( !InState(st_waitActivation, "Activated Dss") )return;
            
            if (from ==&m_pOwner->m_netDss.GetStack())
            {
                ESS_ASSERT(!m_netSideIsActive);                
                m_netSideIsActive = true;
                if (m_userSideIsActive)
                {
                    m_state = st_null;
                    m_pOwner->DssActivated(); // to DssScenario                
                }               
            }
            else if (from == &m_pOwner->m_userDss.GetStack())
            {
                ESS_ASSERT(!m_userSideIsActive);                
                m_userSideIsActive = true;
                if (m_netSideIsActive)
                {
                    m_state = st_null;
                    m_pOwner->DssActivated(); // to DssScenario               
                }
            }
            else ESS_ASSERT(0 && "Bad IsdnStack* ");                                        
            
        }

        void Deactivated(IsdnStack* from) 
        {
            if (from == &m_pOwner->m_userDss.GetStack())
            {
                m_userSideIsActive = false;
            }
            else
            if (from == &m_pOwner->m_netDss.GetStack())
            {
                m_netSideIsActive = false;
            }
            else
                ESS_ASSERT(m_state == st_null && "Bad IsdnStack* ");
            
            //TODO if both side deactivated
            if (!m_userSideIsActive && !m_netSideIsActive) m_pOwner->DssDeactivated(from);
        }
        
        void DssErrorMsg(shared_ptr<const DssWarning> pWarning, IsdnStack* from) 
        {
            //TUT_ASSERT(st_waitActivation == m_state);
            //TUT_ASSERT(++m_tryCount <= CMaxTryCount);
            //if (  dynamic_cast<const ISDN::DssNotDeactivated*>(pWarning.get()) != 0  )
            //{
            //    //try reactivate:
            //    m_pOwner->m_leftDss.GetStack().GetDssIntf()->Activate();                
            //}
        }

        void RestartComplete(bool ok, const std::string & info, ISDN::IsdnStack *from)
        {
            TUT_ASSERT(m_waitRestartCompleteFrom == from && "Unexpected RestartComplete");
            TUT_ASSERT(ok && "RestartFailed");
            m_waitRestartCompleteFrom = 0;
            if (!m_restartCompleteAction) return;
            
            Utils::VIExecuter::Exec(m_restartCompleteAction);
            m_restartCompleteAction = 0;                                    
        }

    public:

        void ActivateDss(IsdnStack* pStack)
        {
            ESS_ASSERT( m_userSideIsActive == false && m_netSideIsActive == false);
            
            ISDN::ILayerDss* pDss = pStack->GetL3Ptr(); //TODO - GetILayerDss
            pDss->Activate();
            if (pStack == &m_pOwner->m_netDss.GetStack() || pStack == &m_pOwner->m_userDss.GetStack())
                m_state = st_waitActivation;
            
            ESS_ASSERT(m_state == st_waitActivation && "Bad IsdnStack* " );
        }

        void RestartReq(bool user, Utils::IVirtualInvoke *pInvoke)
        {                 
            
            if (user)
            {
                ESS_ASSERT(m_userSideIsActive);
                m_waitRestartCompleteFrom = &m_pOwner->m_userDss.GetStack();
            }
            else
            {
                ESS_ASSERT(m_netSideIsActive);
                m_waitRestartCompleteFrom = &m_pOwner->m_netDss.GetStack();                      
            }

            ISDN::ILayerDss* pDss = m_waitRestartCompleteFrom->GetL3Ptr();
            pDss->RestartReq();
            m_restartCompleteAction = pInvoke;            
        }
       
        Fsm(DssScenario* pOwner) 
            : m_pOwner(pOwner),
            m_state(st_null),
            m_userSideIsActive(false),
            m_netSideIsActive(false),
            m_waitRestartCompleteFrom(0),
            m_restartCompleteAction(0)
            //m_tryCount(0)
        {}
    };

    //...

    
    DssScenario::DssScenario(DomainClass &domain, NObjDssTestSettings& prof,
                             ITestResult& callback, const std::string& traceName, bool autoStartPolling)
        : MsgObject( domain.getMsgThread() ),        
        m_dssTestProf(prof),
        m_callback(callback),
        //m_logPrepear(prof.getLogOutSettings(), shared_ptr<iLogW::ILogRecordFormater>(new iLogW::MsgBlockFormater)),
        m_logSession(traceName, domain.Log().getSessionInterface(), prof.m_TraceTest, domain.Log().getLogSessionProfile()),
        m_logTags(m_logSession),
        m_infra(&m_rnd, domain.Log().getSessionInterface(), domain.Log().getLogSessionProfile()),
        m_pFsm( new Fsm(this) ),
        m_pollingTimer(this, &DssScenario::OnPollingTimer),
        m_autoStartpolling(autoStartPolling),
        m_domain(domain),               
        
        m_pLapdPrifile(CreateLapdTestProfile(m_dssTestProf)),        

        m_userIntfGroup(0, ISDN::SetBCannels("1..15, 17..31")),
        m_netIntfGroup (0, ISDN::SetBCannels("1..15, 17..31")),

        m_userStProf(L2Profile::CreateAsTE(*prof.m_pL2TraceOption), 
            L3Profile::CreateAsUser(*prof.m_pL3TraceOption, &m_userIntfGroup)),
        m_netStProf(L2Profile::CreateAsNT(*prof.m_pL2TraceOption), 
            L3Profile::CreateAsNet(*prof.m_pL3TraceOption, &m_netIntfGroup)),    
            
        m_userDss(domain, m_infra, m_userStProf, *m_pLapdPrifile, new ISDN::LeftDrvRole, m_pFsm.get(), m_inCallRouter),
        m_netDss(domain, m_infra, m_netStProf, *m_pLapdPrifile, new ISDN::RightDrvRole, m_pFsm.get(), m_inCallRouter),

        m_timeout(this, &DssScenario::OnTimeout)
    {
        m_userIntfGroup.SetStrategy<ISDN::OrderF>();
        m_netIntfGroup.SetStrategy<ISDN::OrderB>();

        m_userDss.ComunicateOpposite(m_netDss);
        m_netDss.ComunicateOpposite(m_userDss);        

        m_rnd.setSeed( static_cast<int>(m_infra.GetTick()) );
    }

    DssScenario::~DssScenario()
    {                           
        ISDN::L3StatData l3statRight = getRightDssStat();
        m_logSession.Log(l3statRight.getAsString(), m_logTags.RDssStat);

        ISDN::L3StatData l3statLeft = getLeftDssStat();
        m_logSession.Log(l3statLeft.getAsString(), m_logTags.LDssStat);

        ISDN::L2StatData l2statLeft = m_userDss.GetStack().getL2Stat();
        m_logSession.Log(l2statLeft.getAsString(), m_logTags.LLapdStat);

        ISDN::L2StatData l2statRight = m_netDss.GetStack().getL2Stat();
        m_logSession.Log(l2statRight.getAsString(), m_logTags.RLapdStat);
    }    

    ISDN::L3StatData DssScenario::getLeftDssStat()
    {
        return m_userDss.GetStack().getL3Stat();
    }

    ISDN::L3StatData DssScenario::getRightDssStat()
    {
        return m_netDss.GetStack().getL3Stat();
    }

    

    void DssScenario::AsyncRun() 
    {           
        PutMsg(this, &DssScenario::OnRun);

        //m_pFsm->ActivateDss( &m_leftTEStack );       
    }

    void DssScenario::OnRun() 
    {           
        m_pFsm->ActivateDss( &m_userDss.GetStack() ); 
        if (m_dssTestProf.m_MaxTestTimeout > 0) m_timeout.Start(m_dssTestProf.m_MaxTestTimeout);
    }

    Domain::IDomain& DssScenario::GetIDomain()
    {
        return m_domain;
    }

    IsdnStack& DssScenario::GetLeftTEstack()
    {
        return m_userDss.GetStack();
    }

    IsdnStack& DssScenario::GetRightNTstack()
    {
        return m_netDss.GetStack();
    }

//     iLogW::LogStore& DssScenario::GetLogStore()
//     {
//         return m_domain.Log().Store();
//     }

    IsdnInfraTest& DssScenario::GetInfra()
    {
        return m_infra;
    }

    Utils::Random& DssScenario::GetRandom()
    {
        return m_rnd;
    }

    void DssScenario::ReloadDriver()//(NLayer<1>)
    {        
        m_logSession.Log("Reload Driver"/*, cmd from subs*/);
        m_userDss.ReloadDrv();            
        m_netDss.ReloadDrv();        
    }

    void DssScenario::ReloadL2()//ReloadLayer(NLayer<2>)
    {     
        m_logSession.Log("Reload IsdnL2"/*, cmd from subs*/);
        m_userDss.GetStack().ReloadL2(m_userStProf.m_L2prof);
        m_netDss.GetStack().ReloadL2(m_netStProf.m_L2prof);
    }

    void DssScenario::ReloadL3()//ReloadLayer(NLayer<3>)
    {     
        m_logSession.Log("Reload IsdnL3"/*, cmd from subs*/);
        m_userDss.GetStack().ReloadL3(m_userStProf.m_L3prof);
        m_netDss.GetStack().ReloadL3(m_netStProf.m_L3prof);
    }    

    void DssScenario::ReloadUser()//ReloadLayer(NLayer<4>)
    {     
        m_logSession.Log("Reload User"/*, cmd from subs*/);
        m_userDss.ReloadUser();        
        //m_rightDss.ReloadUser();        
    }
    
    void DssScenario::FinishTest()
    {            
        //m_domain.Stop(Domain::DomainExitOk);
        m_callback.TestFinished(true, "TestComplete");
        m_timeout.Stop();
    }

    void DssScenario::StartPolling()
    {
        ESS_ASSERT(!m_autoStartpolling);
        m_pollingTimer.Start(CPollInterval, true);
    }

    const iLogW::LogSessionProfile& DssScenario::GetLogSessionProf()
    {
        return m_domain.Log().getLogSessionProfile();
    }

    iLogW::ILogSessionToLogStore& DssScenario::GetLogStoreIntf()
    {
        return m_domain.Log().getSessionInterface();
    }

    void DssScenario::RestartReq(bool userSide, Utils::IVirtualInvoke *pInvoke)
    {        
        m_pFsm->RestartReq(userSide, pInvoke);
    }
} // namespace IsdnTest

