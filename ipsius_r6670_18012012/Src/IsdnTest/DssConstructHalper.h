#ifndef DSSCONSTRUCTHALPER_H
#define DSSCONSTRUCTHALPER_H

#include "Domain/DomainClass.h"

#include "ISDN/IsdnStack.h"
#include "ISDN/IsdnStackSettings.h"
#include "ISDN/IsdnRole.h"

#include "ISDN/L2Profiles.h"
#include "ISDN/L3Profiles.h"

#include "DriverL1Test.h"
#include "DssCallBackAdapter.h"
#include "DrvLinkLayer.h"


namespace IsdnTest
{
    // Объект сборки/линковки одной Dss-системмы  для теста на основе DssScenario
    class DssConstructHelper
    {
        //DssCallBackAdapter m_adapter;
        scoped_ptr<DssCallBackAdapter> m_adapter;
        // for User reload
        IScnFsm* m_pFsm;
        IncommingCallRouter& m_inCallRouter;

        scoped_ptr<IsdnRole> m_pDrvRole;

        ISDN::IsdnStackSettings& m_stackProf;                   

        scoped_ptr<DriverL1Test> m_pDrv;
        //for drvReload
        DomainClass& m_domain;
        IsdnInfraTest& m_infra;        
        const NObjLapdTestSettings& m_drvProf;

        ISDN::IsdnStack m_stack;

        DrvLinkLayer m_p2p;        

    public:

        
        DssConstructHelper(DomainClass& domain, IsdnInfraTest& infra, IsdnStackSettings& stackProf,
                           const NObjLapdTestSettings& drvProf, ISDN::IsdnRole* role,
                           IScnFsm* pFsm, IncommingCallRouter& inCallRouter)
            : m_adapter(new DssCallBackAdapter(domain, pFsm, inCallRouter)),
            m_pFsm(pFsm),
            m_inCallRouter(inCallRouter),
            m_pDrvRole(role),
            m_stackProf(stackProf),            
            m_pDrv(new DriverL1Test(domain, &infra, *m_pDrvRole, drvProf)), 
            m_domain(domain),
            m_infra(infra),            
            m_drvProf(drvProf),
            m_stack(domain, stackProf, infra, m_pDrv->GetBinder(), m_adapter->GetBinder()),
            m_p2p(m_pDrv.get(), domain.getMsgThread())            
        {
            ESS_ASSERT(pFsm);
            m_pDrv->SetDownIntf(&m_p2p);            
            m_adapter->SetStack(&m_stack);
        }

        void ComunicateOpposite(DssConstructHelper& rhs)
        {
            m_p2p.SetOpposite(&rhs.m_p2p);            
        }

        ISDN::IsdnStack& GetStack()
        {
            return m_stack;
        }

        void ReloadDrv()
        {
            m_pDrv.reset( 
                new DriverL1Test(m_domain, &m_infra, *m_pDrvRole, m_drvProf, m_stack.GetL2Ptr()->GetBinderIDrvToL2())                
            );

            m_pDrv->SetDownIntf(&m_p2p);
            m_p2p.SetNewDrv(m_pDrv.get());
            
            m_stack.SetNewDriver( m_pDrv->GetBinder() );
        }

        void ReloadUser()
        {
            m_adapter.reset( new DssCallBackAdapter(m_domain, m_pFsm, m_inCallRouter) );
            m_adapter->SetStack(&m_stack);
            m_stack.SetNewUser( m_adapter->GetBinder() );
        }

    };

} // IsdnTest

#endif

