#ifndef DUMMYLAYER3_H
#define DUMMYLAYER3_H

#include "isdn/IsdnLayersInterfaces.h"

#include "ObjLink/ObjectLinksServer.h"

#include "Domain/DomainClass.h"

#include "logger.h"

namespace IsdnTest
{
    using Domain::DomainClass;

    class DummyLayer3
        : public ISDN::IL2ToL3,
        public iCore::MsgObject,
        public ObjLink::IObjectLinksHost,
        boost::noncopyable
    {
        Logger m_session;

        ObjLink::ObjectLinksServer m_server;
        DomainClass &m_domain;  

        ISDN::IL3ToL2 * m_pIntfDown;

    //IL2ToL3 implementation
    private:

        void EstablishInd()
        {
            m_session.Log("EstablishInd");
        }

        void EstablishConf()
        {
            m_session.Log("EstablishConf");
        }

        void ReleaseConf()
        {
            m_session.Log("ReleaseConf");
        }

        void ReleaseInd()
        {
            m_session.Log("-----------------\nReleaseInd. Send EstablishReq()\n-------------------");
            m_pIntfDown->EstablishReq();
        }

        void DataInd(ISDN::IPacket *pPacket)
        {
            m_session.Log("DataInd/* - return packet to IsdnL2*/");
            
            //m_pIntfDown->DataReq( pPacket );
            
        }

        void UDataInd(ISDN::UPacket *pPacket)
        {            
            m_session.Log("UdataInd");
        }

        void SetDownIntf(ISDN::BinderToIL3ToL2 binderToL2)
        {
            m_session.Log("SetDownIntf");
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

    public:

        DummyLayer3(DomainClass & domain, const Logger& parentSession)
            : MsgObject(domain.getMsgThread()),
            m_session(parentSession, this),
            m_server(domain, this),
            m_domain(domain),
            m_pIntfDown(0)
        {}

        void SetDownIntf(ISDN::IL3ToL2 * pIntfDown)
        {
            TUT_ASSERT(m_pIntfDown = pIntfDown);
        }

        void Run()
        {
            //ESS_ASSERT(m_pIntfDown);
            //m_pIntfDown->EstablishReq();
        }
    };
} // namespace IsdnTest

#endif
