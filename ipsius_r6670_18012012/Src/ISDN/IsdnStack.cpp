#include "stdafx.h"
#include "Domain/DomainClass.h"
#include "IsdnStack.h"

#include "isdnl2.h"
#include "isdnl3.h"


namespace ISDN
{    

    IsdnStack::IsdnStack(IDomain& iDomain, const IsdnStackSettings& prof, IsdnInfra& infr, BinderToIDriver drv, BinderToICallBackDss pUpIntf)
        : m_iDomain(iDomain),        
        m_drvBinder(drv),
        m_usrBinder(pUpIntf),      
        m_pL2( new IsdnL2(iDomain.getDomain(), drv, &infr, prof.m_L2prof) ),
        m_pL3( new IsdnL3(iDomain, infr, *prof.m_L3prof) ),
        m_infr(infr) 
    {              
        m_pL2->SetUpIntf(m_pL3.get(), m_pL3.get());
        m_pL3->SetL2Link(m_pL2.get(), m_pL2.get());
        ESS_ASSERT(pUpIntf);
        m_pL3->SetMngLink(pUpIntf);        

        Start();
    }

    IsdnStack::~IsdnStack()
    {
        Stop();
        //std::cout << "*******Destructor IsdnStack\n";
    }

	/*IL2ToDriver* IsdnStack::GetDrvPtr()
	{
        return m_pDrv;
    }*/

	IsdnL2* IsdnStack::GetL2Ptr()
	{
        ESS_ASSERT( m_pL2.get() != 0 );
        return m_pL2.get();
    }

	IsdnL3* IsdnStack::GetL3Ptr()
	{
        return m_pL3.get();
    }

    ILayerDss* IsdnStack::GetDssIntf()
    {
        return m_pL3.get();
    }

    IeConstants::Location IsdnStack::GetLocation()
    {
        return m_pL3->GetLocation();
    }

    void IsdnStack::Start(int period)
    {
        m_pL2->StartProcessing(period);        
    }

    void IsdnStack::Stop()
    {
        m_pL2->StopProcessing();        
    }

    void IsdnStack::SetNewDriver(BinderToIDriver drvBinder)
    {
        m_drvBinder = drvBinder;
        m_pL2->SetDownIntf(drvBinder);
    }

    void IsdnStack::ReloadL2(const shared_ptr<const L2Profile> newProfile)
    {
        //bool traceOn = m_pL2->getTraceOn();
        m_pL2.reset( 
            new IsdnL2(m_iDomain.getDomain(), m_drvBinder, m_pL3->GetBinder<IL2ToL3>(),
                           &m_infr, newProfile) 
        );
    }

    void IsdnStack::ReloadL3(const shared_ptr<const L3Profile> newProfile)
    {        
        m_pL3.reset(
            new IsdnL3(m_iDomain.getDomain(), m_infr, *newProfile,
                              m_usrBinder, m_pL2.get(), m_pL2.get())
        );
    }

    void IsdnStack::SetNewUser(BinderToICallBackDss userBinder)
    {
        m_usrBinder = userBinder;
        m_pL3->SetMngLink(userBinder);
    }

    L2StatData IsdnStack::getL2Stat() const
    {
        return m_pL2->getStatData();
    }

    void IsdnStack::ClearL2Stat()
    {
        m_pL2->ClearStat();
    }

    L3StatData IsdnStack::getL3Stat() const
    {
        return m_pL3->getStatData();
    }

    void IsdnStack::ClearL3Stat()
    {
        m_pL3->ClearStat();
    }

} // namespace ISDN

