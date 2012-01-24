#ifndef YATEL3TESTOBJ_H
#define YATEL3TESTOBJ_H

#include "Domain/DomainClass.h"

#include "iCore/MsgObject.h"
#include "iCore/MsgTimer.h"

#include "iLog/LogWrapper.h"

#include "Utils/Random.h"

#include "YateL3TestProfile.h"
#include "LogFormating.h"
#include "InfraTest.h"
#include "L1UdpDriver.h"
#include "UdpChannel.h"
#include "DssUserForYate.h"

#include "ISDN/IsdnStack.h"



namespace IsdnTest
{

    using Domain::DomainClass;
    using iCore::MsgObject;
    using ISDN::IsdnStack;

    class YateL3TestObject : public MsgObject
    {
        enum {CPullingInterval = 100};
        typedef YateL3TestObject  TMy;

        DomainClass& m_domain;
        iLogW::LogStore& m_logStore;
        iCore::MsgTimer m_tPull;
        SessionHandler m_sessionHandler;
        iLogW::LogSessionProfile m_totalLogSessionProf;
        Logger m_logSession;

        Utils::Random m_rnd;
        IsdnInfraTest m_infra;
        L1UdpDriver m_drv;
        UdpChannel m_dataChannel;

        DssUserForYate m_dssUser;
        IsdnStack m_stack;

        void Run()
        {
            m_tPull.Start(CPullingInterval, true);
        }

        void OnPull(iCore::MsgTimer*)
        {
            if (m_dssUser.IsComplete())
            {
                m_logSession.Log("DssForYate Test Complete.");
                m_domain.Stop();
            }
        }

    public:

        YateL3TestObject(DomainClass& domain, YateL3TestProfile &prof)
            : MsgObject(domain.getMsgThread()),
            m_domain(domain),
            m_tPull(this, &YateL3TestObject::OnPull),
            m_logStore( prof.getLogStore() ),
            m_sessionHandler(prof),
            m_totalLogSessionProf(false, &m_sessionHandler),
            m_logSession("YateL2TestObj", m_logStore.getSessionInterface(), prof.getTraceOn(), m_totalLogSessionProf),
            m_infra(&m_rnd, m_logStore.getSessionInterface(), m_totalLogSessionProf),
            m_drv(domain, &m_infra, m_logSession),
            m_dataChannel( domain.getMsgThread(), prof.getCfgHost() ),
            m_dssUser(domain, m_logSession, m_infra, prof),
            m_stack(domain, prof.getStackProf(), &m_infra, m_drv.GetBinder(), m_dssUser.getICallBackDssBinder())
        {
            m_dataChannel.SetDrv(&m_drv);
            m_drv.SetDownIntf(&m_dataChannel);
            m_dssUser.SetDownIntf( m_stack.GetL3Ptr()->GetBinder<ISDN::ILayerDss>() );

            PutMsg(this, &TMy::Run);
        }
    };

}//namespace IsdnTest

#endif
