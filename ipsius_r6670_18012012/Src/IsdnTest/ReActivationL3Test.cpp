#include "stdafx.h"
#include "iLog/LogSessionProfileSetup.h"
#include "iLog/LogWrapperLibrary.h"
#include "ReActivationL3Test.h"

#include "DriverDummy.h"
#include "infratest.h"
#include "ISDN/IsdnStack.h"
#include "ISDN/IsdnIntfGroup.h"
#include "iLog/LogManager.h"

#include "ITestResult.h"

namespace IsdnTest
{
   
    using Domain::DomainClass;
    using boost::scoped_ptr;

    //-------------------------------------------------------------------------------------

    class ReActivationL3Test::Impl : boost::noncopyable,
        public iCore::MsgObject,
        public ISDN::ICallbackDss, 
        public ObjLink::IObjectLinksHost   // server
    {        
        typedef ISDN::ICallbackDss TMyIntf;
        typedef ObjLink::ObjLinkBinder<TMyIntf> BindToMy;

        ITestResult& m_callback;
        NObjReActivationL3TestSettings& m_prof;

        ObjLink::ObjectLinksServer m_server;//server
        DomainClass &m_domain;     //server        
        ObjLink::ObjLinkStoreBinder m_myLinkBinderStorage;        
                
        Utils::Random m_rnd;
        iLogW::ILogSessionToLogStore& m_logStore; 
        //iLogW::LogSessionProfileSetup m_logSetup;
        IsdnInfraTest m_infra;        
        DriverDummy m_drv;

        ISDN::IsdnIntfGroup m_intfGroup;
        scoped_ptr<ISDN::IsdnStackSettings> m_isdnProf;
        scoped_ptr<ISDN::IsdnStack> m_stack;        

        enum State
        {
            st_null,
            st_waitWarning,
            st_waitDeactivate,
            //st_complete
        };

        State m_state;
        iCore::MsgTimer m_t;

        void OnTimer(iCore::MsgTimer*)
        {
            m_state = st_waitWarning;
        }

        void RunTest()
        {
            m_stack->GetDssIntf()->Activate();
            m_t.Start(m_prof.m_ReActivationInterval * m_prof.m_ReActivationCount);
        }

    // ICallbackDss impl:
    private:
        void Activated()  { TUT_ASSERT(0); }

        //-------------------------------------------------------------------------------------

        void Deactivated() 
        {
            TUT_ASSERT(m_state == st_waitDeactivate && "Unexpected deactivate");
            m_callback.TestFinished(true, "Complete");			
        }

        //-------------------------------------------------------------------------------------

        void IncomingCall(IncommingCallParametrs params) { TUT_ASSERT(0); }

        //-------------------------------------------------------------------------------------

        void DssErrorMsg(shared_ptr<const ISDN::DssWarning> pWarning)
        {
            if (m_state != st_waitWarning) return;

            if (dynamic_cast<const ISDN::L2NotActve*>(pWarning.get()))
            {
                m_state = st_waitDeactivate;
                return;
            }

            TUT_ASSERT(0 && "Unexpected warning");
        }

        //-------------------------------------------------------------------------------------

        void RestartComplete(CompleteInfo info)
        {
            ESS_UNIMPLEMENTED;
        }

    public:

        Impl(Domain::DomainClass &domain, NObjReActivationL3TestSettings &prof, ITestResult& callback) : 
            MsgObject(domain.getMsgThread()),
            m_callback(callback),
            m_prof(prof),
            m_server(domain, this),
            m_domain(domain),            
            m_logStore(domain.getDomain().Log().getSessionInterface()),                          
            m_infra(&m_rnd, m_logStore, domain.Log().getLogSessionProfile()),
            m_drv(domain),
            m_intfGroup(0, ISDN::SetBCannels("1..15, 17..31")),
            m_state(st_null),
            m_t(this, &Impl::OnTimer)
        {
            boost::shared_ptr<ISDN::L3Profile> l3Prof = ISDN::L3Profile::CreateAsUser(*prof.m_pL3TraceOption, &m_intfGroup);
            l3Prof->GetTimers().tReActivateReqL3 = ISDN::DssTimerValue(m_prof.m_ReActivationInterval * m_prof.m_ReActivationCount);

            m_isdnProf.reset( new ISDN::IsdnStackSettings( ISDN::L2Profile::CreateAsTE(*prof.m_pL2TraceOption), l3Prof ));
            m_stack.reset( new ISDN::IsdnStack(domain, *m_isdnProf, m_infra, m_drv.GetBinder(), GetBinder()));

            PutMsg(this, &Impl::RunTest);
        }

        boost::shared_ptr<BindToMy> GetBinder()
        {
            return m_myLinkBinderStorage.getBinder<TMyIntf>(this);
        }

    // IObjectLinksHost impl
    private:

        Utils::SafeRef<ObjLink::IObjectLinksServer> getObjectLinksServer()
        {
            return m_server.getInterface();
        }

    // IDomain impl
    private:

        DomainClass& getDomain()
        {
            return m_domain;
        }   
    };

    //-------------------------------------------------------------------------------------

    ReActivationL3Test::ReActivationL3Test(Domain::DomainClass &domain, NObjReActivationL3TestSettings &profile, ITestResult& callback)
        : m_impl( new Impl(domain, profile, callback))
    {
    }

    //-------------------------------------------------------------------------------------

    ReActivationL3Test::~ReActivationL3Test()
    {}

} // namespace IsdnTest

