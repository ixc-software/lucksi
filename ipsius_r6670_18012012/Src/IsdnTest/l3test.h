#ifndef L3TEST_H
#define L3TEST_H

#include "stdafx.h"
#include "Domain/DomainClass.h"
#include "ObjLink/ObjectLinksServer.h"
#include "ISDN/IsdnLayersInterfaces.h"
#include "Utils/ErrorsSubsystem.h"
#include "Platform/Platform.h"


namespace IsdnTest
{
    using Platform::byte;

	class L3State;
	class IL3Test
	{
	public:

		virtual void SwitchTo(L3State*) = 0;
        virtual ISDN::IL3ToL2 *GetIntfDown() = 0;        
        virtual ~IL3Test(){}
	};

    //------------------------------------

	class L3State:public ISDN::IL2ToL3
	{
        int i;
	public:
		
        L3State(IL3Test* pL3): i(0), m_pL3(pL3),  m_complete(false) {}
		
        virtual ~L3State() {}
		
        void EstablishInd() { OnEmptyEvent(); } // override
		void EstablishConf() { OnEmptyEvent(); } // override
			
		void ReleaseConf() { OnEmptyEvent(); } // override
		void ReleaseInd() { OnEmptyEvent(); } // override
			
		void DataInd(QVector<byte>) { OnEmptyEvent(); } // override
		void UDataInd(QVector<byte>){ OnEmptyEvent(); } // override

        void SetDownIntf(ISDN::BinderToIL3ToL2) {TUT_ASSERT (0 && "CallUnusedMethod");} // override

		virtual void OnEmptyEvent() = 0;

        void ErrorInd(ISDN::L2Error error)
        {
            // nothing TODO (unsupported)
            if (error.getErrorCode() == 'F') ++i;
            if (i == 2)
                std::cout << "break";
        }


        bool Complete() 
        {
            //bool ret = m_complete;
            //m_complete = false; // защита от повторного запроса (забыл установить состояние)
            //return ret;
            return m_complete;
        }

		void SwitchState(L3State* newState) // вызов из имплементации сост-теста
		{
			m_pL3->SwitchTo(newState); // переход машины в новое состояние созданное в импл сост-тест
			//delete this; // удаление отработавшего сост-теста - перенесено в SwitchTo
		}
		
        IL3Test* GetL3(){return m_pL3;}

    protected:

        bool m_complete;

	private:

		IL3Test* m_pL3;
		
	};
	
	//-----------------------------------

    using boost::shared_ptr;
    using Domain::DomainClass;    

	
	class L3Test : 
	    public iCore::MsgObject,
        public ISDN::IL2ToL3,
        public IL3Test,        
        public ObjLink::IObjectLinksHost,   //server         
        boost::noncopyable
	{

	public:
		
        L3Test(DomainClass& domain) //: m_pLastPacket(0)
            : MsgObject(domain.getMsgThread()),
            m_server(domain, this),
            m_domain(domain),
			m_pIntfDown(0)
        {
            TUT_ASSERT( (m_pCurrState = GetStartTest())  &&  "Start state invalid");
        }
		 
		~L3Test() 
        {
            delete m_pCurrState;
            //if(m_pLastPacket)m_pLastPacket->Delete();
        }
		
		void SetDownIntf(ISDN::IL3ToL2 * pIntfDown)
        {
            TUT_ASSERT(m_pIntfDown = pIntfDown);
        }


        void SetStateWaitEstConf();

        void SetStateWaitRelInd();

        void SetStateWaitRelConf();
        
        bool Complete() { return m_pCurrState->Complete(); }

        /*ISDN::IsdnPacket* GetLastPacket()
        {
            if(!m_pLastPacket) return 0;
            ISDN::IsdnPacket* ret = m_pLastPacket->Clone();
            m_pLastPacket->Delete();
            m_pLastPacket = 0;
            return ret;
        }*/
		
		void SwitchTo(L3State* newState) // override
        {
            delete m_pCurrState;
            TUT_ASSERT(m_pCurrState = newState);
            //++m_stateCount;
        }

        ISDN::IL3ToL2 *GetIntfDown() {return m_pIntfDown;}

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

    // IL2ToL3 impl
    private:
		
		void EstablishInd() // override
        {
            //std::cout << "EsablishInd" << std::endl;
            TUT_ASSERT(m_pCurrState); m_pCurrState->EstablishInd();
        } 

		void EstablishConf()  // override
        {
            //std::cout << "EsablishConf" << std::endl;
            TUT_ASSERT(m_pCurrState); m_pCurrState->EstablishConf();
        }
			
		void ReleaseConf() // override
        {
            //std::cout << "ReleaseConf" << std::endl;
            TUT_ASSERT(m_pCurrState); m_pCurrState->ReleaseConf();
        }

		void ReleaseInd() // override
        {
            //std::cout << "ReleaseInd" << std::endl;
            TUT_ASSERT(m_pCurrState); m_pCurrState->ReleaseInd();
        }
			
		void DataInd(QVector<byte> l2data)  // override
        {            
            TUT_ASSERT(m_pCurrState);
            //m_pCurrState->DataInd( pPacket->Clone() );
            m_pCurrState->DataInd(l2data);
        }

		void UDataInd(QVector<byte> l2data) // override
        {
            TUT_ASSERT(m_pCurrState); 
            //m_pCurrState->UDataInd( pPacket->Clone() );
            m_pCurrState->UDataInd(l2data);
        }      

        void SetDownIntf(ISDN::BinderToIL3ToL2) {/*nothing TODO*/}

        void ErrorInd(ISDN::L2Error error)
        {
            // nothing TODO
        }

	private:

         L3State* GetStartTest(); // overide

		// входной и-фейс 2ур
		ISDN::IL3ToL2 *m_pIntfDown;
		L3State* m_pCurrState;        // TODO change to scoped_ptr, remove asserts for null 
        //ISDN::IsdnPacket* m_pLastPacket;

        ObjLink::ObjectLinksServer m_server;   // server
        DomainClass &m_domain;                 // server     
       
	};

}//namespace IsdnTest

#endif
