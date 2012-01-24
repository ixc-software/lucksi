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
#include "Utils/ErrorsSubsystem.h"
#include "ISDN/IsdnLayersInterfaces.h"
#include "ISDN/PacketViewer.h"
#include "logger.h"
#include "LogOut.h"
#include "ISDN/IsdnRole.h"
#include "NObjLapdTestSettings.h"



namespace IsdnTest
{
    using Domain::DomainClass;
    using iCore::MsgObject;

    using ObjLink::ILinkKeyID;
    using ObjLink::ObjectLinkError;
    using ObjLink::ObjectLink;

    using boost::shared_ptr;  

    using Platform::byte;

	class IDrvLinkLayer
	{
	public:

		virtual void SendData(QVector<byte> packet) = 0;
        virtual void Activate() = 0;
        virtual void Deactivate() = 0;
        virtual ~IDrvLinkLayer(){};

	};

	class DriverL1Test
        : public ISDN::IL2ToDriver,
        public MsgObject,
        public ObjLink::IObjectLinksHost,   // server 
        public ObjLink::IObjectLinkOwner    // client
	{
        typedef ISDN::IL2ToDriver TMyIntf;
        typedef ObjLink::ObjLinkBinder<TMyIntf> BindToMy;
        typedef ObjLink::ObjLinkStoreBinder BindToMyStorage;        

	public:

        DriverL1Test(DomainClass& domain, ISDN::IsdnInfra* pInfra, 
            ISDN::IsdnRole& role, const NObjLapdTestSettings& prof) :   
          MsgObject( domain.getMsgThread() ),          
          m_pIntfDown(0),
          m_nameObj(role.Name()),
          m_log(role.Name(), pInfra->getLogStoreIntf(), prof.m_TracedTest, pInfra->getLogSessionProfile()),          
          m_active(false),          
          m_kill(false),          
          m_role(role),
          m_PercentKilled(prof.m_DropRate),
          m_prof(prof),
          m_server( domain, this ),
          m_domain( domain ),
          m_pIntfUp(*this),
          m_recreated(false)
          {}

          // конструктор перезагрузки
          DriverL1Test(DomainClass& domain, ISDN::IsdnInfra* pInfra, 
              ISDN::IsdnRole& role, const NObjLapdTestSettings& prof, ISDN::BinderToIDrvToL2 pIntfUp) :   
          MsgObject( domain.getMsgThread() ),
              m_pIntfUp(*this),
              m_pIntfDown(0),
              m_nameObj(role.Name()),
              m_log(role.Name(), pInfra->getLogStoreIntf(), prof.m_TracedTest, pInfra->getLogSessionProfile()),
              //m_PV(role.IsLeft(), *pInfra, prof.TraceRaw ),
              m_active(false),              
              m_kill(false),// m_prev(true),                   
              m_role(role),
              m_PercentKilled(prof.m_DropRate),
              m_prof(prof),
              m_server( domain, this ),
              m_domain( domain ),
              m_recreated(true)
          {
              SetUpIntf(pIntfUp);
          }

       

        
		
		void SetUpIntf(ISDN::BinderToIDrvToL2 pIntfUp)
        {   
            if ( m_pIntfUp.Connected() ) return; // Это должно быть в Л2
            ESS_ASSERT ( pIntfUp->Connect(m_pIntfUp) );
            //m_pIntfUp = pIntfUp;
        }

        boost::shared_ptr<BindToMy> GetBinder()
        {
            return m_myLinkBinderStorage.getBinder<TMyIntf>(this);
        }

        void SetDownIntf(IDrvLinkLayer* pIntfDown)
        {
            TUT_ASSERT(pIntfDown);
            m_pIntfDown = pIntfDown;
        }

        void PacketKillOn() {m_kill = true; m_random.setSeed(1);}

        void PacketKillOff() {m_kill = false;}      				

        //receive packet from DrvLinkLayer and send to l2
        void ReciveData(QVector<byte> packet)
        {
            if(m_active)
            {
                TUT_ASSERT(m_pIntfUp.Connected());
                m_pIntfUp->DataInd(packet);
            }
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
		void DataRequest(QVector<byte> packet)
        {
            Writer(m_log).Write() << "DataRequest";           
            //SendOpposite( pPacket->Clone() );
            SendOpposite( packet );
        }                

		void ActivateRequest()                 		
        {                        
            TUT_ASSERT( m_pIntfUp.Connected() );
            m_pIntfUp->ActivateConf();

            Writer(m_log).Write() << "ActivateRequest";
            /*синхронизация и активация*/
            ActivateOpposite();
            m_active = true;
            Writer(m_log).Write() << "Driver activated.";

            m_pIntfUp->ActivateInd();
        }

		void DeactivateRequest() 								
        {
            Writer(m_log).Write() << "DeactivateRequest";
            DeactivateOpposide();
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

		void PullInd(QVector<byte> packet) 				
        {
            Writer(m_log).Write() << "PullInd ";
            /*Если канал свободен, иначе добавляет в свою очередь*/
            //DataRequest( pPacket/*->Clone()*/ ); // там клонируется
            DataRequest(packet);
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
                DeactivateRequest();
                return;
            }
            TUT_ASSERT(0 && "Unexpected linkID");
        }

        void OnObjectLinkError(shared_ptr<ObjectLinkError> error) 
        {
        }        


	private:

		void SendOpposite(QVector<byte> packet)
        {             
            if(m_kill && ( m_random.Next(100) > 100 - m_PercentKilled )/* && m_prev*/) 
            {
                Writer(m_log).Write() << "Drop packet";
                //pPacket->Delete();
                packet.clear();
                return;
            }   
            TUT_ASSERT(m_pIntfDown);
            m_pIntfDown->SendData(packet);
        }
        
        void ActivateOpposite() { m_pIntfDown->Activate();}
        void DeactivateOpposide() {m_pIntfDown->Deactivate(); }
        
		IDrvLinkLayer *m_pIntfDown;
        std::string m_nameObj;
        //ISDN::ILoggable m_log;
        Logger m_log;
        //ISDN::PacketViewer m_PV;
        bool m_active;                
        bool m_kill;//, m_prev;                
        Utils::Random m_random;
        ISDN::IsdnRole &m_role;
        int m_PercentKilled;
        const NObjLapdTestSettings &m_prof;

        
        ObjLink::ObjectLinksServer m_server;//server
        DomainClass &m_domain;     //server        
        BindToMyStorage m_myLinkBinderStorage;
        ObjLink::ObjectLink<ISDN::IDriverToL2> m_pIntfUp;
        bool m_recreated;
	};

}//IsdnTest

#endif
