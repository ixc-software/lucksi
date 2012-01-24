#ifndef YATEL2TESTOBJ_H
#define YATEL2TESTOBJ_H

#include "Domain/DomainClass.h"

#include "iCore/MsgObject.h"
#include "iCore/MsgThread.h"

#include "iLog/LogWrapper.h"

#include "Utils/Random.h"

#include "YateL2TestProfile.h"
#include "LogFormating.h"
#include "InfraTest.h"
#include "DummyLayer3.h"
#include "L1UdpDriver.h"
#include "UdpChannel.h"

#include "ISDN/IsdnL2.h"

namespace IsdnTest
{
    using Domain::DomainClass;
    using iCore::MsgObject;
    using iCore::MsgThread;
    using ISDN::IsdnL2;

    class YateL2TestObj : public MsgObject
    {
        typedef YateL2TestObj TMy;

        DomainClass& m_domain;
        iLogW::LogStore& m_logStore;
        SessionHandler m_sessionHandler;
        iLogW::LogSessionProfile m_totalLogSessionProf;
        Logger m_logSession;

        Utils::Random m_rnd;
        IsdnInfraTest m_infra;
        L1UdpDriver m_drv;
        UdpChannel m_dataChannel;
        IsdnL2 m_layer2;
        DummyLayer3 m_layer3;

        void Run()
        {
            m_layer2.StartProcessing();
            m_layer3.Run();
        }

    public:

        YateL2TestObj(DomainClass& domain, YateL2TestProfile& prof)
            : MsgObject(domain.getMsgThread()), 
            m_domain(domain),
            m_logStore( prof.getLogStore() ),
            m_sessionHandler(prof),
            m_totalLogSessionProf(false, &m_sessionHandler),
            m_logSession("YateL2TestObj", m_logStore.getSessionInterface(), prof.getTraceOn(), m_totalLogSessionProf),
            m_infra(&m_rnd, m_logStore.getSessionInterface(), m_totalLogSessionProf),
            m_drv(domain, &m_infra, m_logSession),
            m_dataChannel( domain.getMsgThread(), prof.getCfgHost() ),
            m_layer2(domain, m_drv.GetBinder(), &m_infra, prof.getL2Prof()),
            m_layer3(domain, m_logSession)
        {
            m_dataChannel.SetDrv(&m_drv);
            m_drv.SetDownIntf(&m_dataChannel);
            m_drv.SetUpIntf( m_layer2.GetBinderIDrvToL2() );
           
            m_layer2.SetUpIntf(&m_layer3, &m_layer3);
            m_layer3.SetDownIntf(&m_layer2);

            PutMsg(this, &TMy::Run);
        }
    };
} //namespace IsdnTest

#endif
