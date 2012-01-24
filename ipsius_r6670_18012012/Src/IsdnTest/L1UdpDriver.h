#ifndef L1UDPDRIVER_H
#define L1UDPDRIVER_H

#include "UdpChannel.h"


#include "stdafx.h"

#include "iCore/MsgObject.h"
#include "ObjLink/ObjectLinkBinder.h"
#include "ObjLink/ObjectLinksServer.h"
#include "Domain/DomainClass.h"

#include "IsdnTestConfig.h"
#include "Utils/ErrorsSubsystem.h"
#include "ISDN/IsdnLayersInterfaces.h"
#include "ISDN/PacketViewer.h"
#include "logger.h"
#include "LogOut.h"
#include "ISDN/IsdnRole.h"

#include "YateL2TestProfile.h"


namespace IsdnTest
{
    using Domain::DomainClass;
    using iCore::MsgObject;

    using ObjLink::ILinkKeyID;
    using ObjLink::ObjectLinkError;
    using ObjLink::ObjectLink;

    using boost::shared_ptr;    

    

    //driver send ActivateInd to L2, after receiving first data
    class L1UdpDriver
        : public ISDN::IL2ToDriver,
        public IChannelToDrv,
        public MsgObject,
        public ObjLink::IObjectLinksHost,   // server 
        public ObjLink::IObjectLinkOwner    // client
    {
        typedef ISDN::IL2ToDriver TMyIntf;
        typedef ObjLink::ObjLinkBinder<TMyIntf> BindToMy;
        typedef ObjLink::ObjLinkStoreBinder BindToMyStorage;        

    public:

        L1UdpDriver(DomainClass& domain, ISDN::IsdnInfra* pInfra, const Logger& parentLog) :   
        MsgObject( domain.getMsgThread() ),          
            m_pIntfDown(0),
            m_nameObj("UdpDriver"),
            m_pInfra(pInfra),
            m_log(parentLog, this),            
            m_active(false),                                                       
            m_server( domain, this ),
            m_domain( domain ),
            m_pIntfUp(*this)            
        {                       
        }

        
        void SetUpIntf(ISDN::BinderToIDrvToL2 pIntfUp)
        {   
            if ( m_pIntfUp.Connected() ) return;
            ESS_ASSERT ( pIntfUp->Connect(m_pIntfUp) );            
        }

        boost::shared_ptr<BindToMy> GetBinder()
        {
            return m_myLinkBinderStorage.getBinder<TMyIntf>(this);
        }

        void SetDownIntf(IDrvToChannel* pIntfDown)
        {
            TUT_ASSERT(pIntfDown);
            m_pIntfDown = pIntfDown;
        }
        				

        //receive packet from DrvLinkLayer and send to l2
        void DataInput(const std::vector<byte>& data) // override
        {          
            if(!m_active)
                ActFromLL();
            
            TUT_ASSERT(m_pIntfUp.Connected());
            
            ISDN::IsdnPacket* pPacket = m_pInfra->CreatePacket<ISDN::IsdnPacket>();
            pPacket->AddBack(data);            

            m_pIntfUp->DataInd(pPacket);
            
        }

        void ActFromLL()
        {
            if(m_active) 
            {
                Writer(m_log).Write() << "Driver already active.";
                return;
            }

            m_active = true;
            Writer(m_log).Write() << "Driver activated.";
            ESS_ASSERT( m_pIntfUp.Connected() );
            m_pIntfUp->ActivateInd();
        }

        void DeactFromLL()
        {
            if(!m_active) 
            {
                Writer(m_log).Write() << "Driver already deactivated.";
                return;
            }
            m_active = false;
            Writer(m_log).Write() << "Driver deactivated.";
            m_pIntfUp->DeactivateInd();
        }


        //Implementation IL2ToDriver:
    private:
        void DataRequest(ISDN::IsdnPacket *pPacket)
        {
            Writer(m_log).Write() << "DataRequest";           
            SendOpposite( pPacket->Clone() );
        }        

        ESS_TYPEDEF(DrvTestException);

        void ActivateRequest()                 		
        {                    
            TUT_ASSERT( m_pIntfUp.Connected() );
            m_pIntfUp->ActivateConf();

            Writer(m_log).Write() << "ActivateRequest";
            /*синхронизация и активация*/
            //ActivateOpposite();
            m_active = true;
            Writer(m_log).Write() << "Driver activated.";

            m_pIntfUp->ActivateInd();
        }

        void DeactivateRequest() 								
        {
            Writer(m_log).Write() << "DeactivateRequest";
            //DeactivateOpposide();
            m_active = false;
            Writer(m_log).Write() << "Driver deactivated";
            if ( m_pIntfUp.Connected() )
                m_pIntfUp->DeactivateConf();
        }

        void PullRequest()  									
        {
            Writer(m_log).Write() << "PullRequest";
            /*Если канал свободен иначе ставит флаг БылЗапрос (в исходном коде)*/
            m_pIntfUp->PullConf();
        }

        void PullInd(ISDN::IsdnPacket *pPacket) 				
        {
            Writer(m_log).Write() << "PullInd ";
            /*Если канал свободен, иначе добавляет в свою очередь*/
            DataRequest( pPacket/*->Clone()*/ ); // там клонируется
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
                //DeactivateRequest();
                return;
            }
            TUT_ASSERT(0 && "Unexpected linkID");
        }

        void OnObjectLinkError(shared_ptr<ObjectLinkError> error) 
        {
        }        


    private:

        void SendOpposite(ISDN::IsdnPacket *pPacket)
        {              
            TUT_ASSERT(m_pIntfDown);
            std::vector<byte> data;
            pPacket->CopyToVector(data);
            pPacket->Delete();

            m_pIntfDown->SendData(data);
        }
      

        IDrvToChannel *m_pIntfDown;
        std::string m_nameObj;
        ISDN::IsdnInfra* m_pInfra;        
        Logger m_log;        
        bool m_active;                                                      
        ObjLink::ObjectLinksServer m_server;//server
        DomainClass &m_domain;     //server        
        BindToMyStorage m_myLinkBinderStorage;
        ObjLink::ObjectLink<ISDN::IDriverToL2> m_pIntfUp;
        bool m_recreated;
    };

} // namespace IsdnTest

#endif
