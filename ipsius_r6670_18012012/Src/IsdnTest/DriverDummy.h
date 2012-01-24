/*
Test implementation DriverL1
*/

#ifndef DRIVERL1TEST_H
#define DRIVERL1TEST_H

#include "stdafx.h"

#include "iCore/MsgObject.h"
#include "ObjLink/ObjectLinkBinder.h"
#include "ObjLink/ObjectLinksServer.h"
#include "Domain/DomainClass.h"

// #include "IsdnTestConfig.h"
#include "DriverL1Test.h"
#include "Utils/ErrorsSubsystem.h"
#include "ISDN/IsdnLayersInterfaces.h"
#include "ISDN/PacketViewer.h"
#include "logger.h"
#include "LogOut.h"




namespace IsdnTest
{
    using Domain::DomainClass;
    using iCore::MsgObject;

    using ObjLink::ILinkKeyID;
    using ObjLink::ObjectLinkError;
    using ObjLink::ObjectLink;

    using boost::shared_ptr;  

    using Platform::byte;

    class DriverDummy
        : public ISDN::IL2ToDriver,
        public MsgObject,
        public ObjLink::IObjectLinksHost,   // server 
        public ObjLink::IObjectLinkOwner    // client
    {
        typedef ISDN::IL2ToDriver TMyIntf;
        typedef ObjLink::ObjLinkBinder<TMyIntf> BindToMy;
        typedef ObjLink::ObjLinkStoreBinder BindToMyStorage;        

        ObjLink::ObjectLinksServer m_server;//server
        DomainClass &m_domain;     //server        
        BindToMyStorage m_myLinkBinderStorage;
        ObjLink::ObjectLink<ISDN::IDriverToL2> m_pIntfUp;

    //IL2ToDriver impl:
    private:

        void SetUpIntf(ISDN::BinderToIDrvToL2 pIntfUp)
        {   
            if ( m_pIntfUp.Connected() ) return;
            ESS_ASSERT ( pIntfUp->Connect(m_pIntfUp) );            
        }

        void DataRequest(QVector<byte> packet) {/*nothing*/}        

        void ActivateRequest() {/*nothing*/}

        void DeactivateRequest() {/*nothing*/}

        void PullRequest() 
        {
            m_pIntfUp->PullConf();
        }

        void PullInd(QVector<byte> packet) {/*nothing*/}

    public:

        DriverDummy(DomainClass& domain) 
            : MsgObject( domain.getMsgThread() ),
            m_server( domain, this ),
            m_domain( domain ),
            m_pIntfUp(*this)
        {}                                                                      

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

        // IObjectLinkOwner impl
    private:

        void OnObjectLinkConnect(ILinkKeyID &linkID) 
        {
            /*if (m_recreated)
            m_pIntfUp->SetDownIntf(m_myLinkBinderStorage.getBinder<ISDN::IL2ToDriver>(this));*/
        }

        void OnObjectLinkDisconnect(ILinkKeyID &linkID) 
        {
            if ( m_pIntfUp.Equal(linkID) )
            {
                //nothing
                return;
            }
            TUT_ASSERT(0 && "Unexpected linkID");
        }

        void OnObjectLinkError(shared_ptr<ObjectLinkError> error) 
        {
        }        
                                                     
    };

}//IsdnTest

#endif
