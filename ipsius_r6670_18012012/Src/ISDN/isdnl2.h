#ifndef ISDNL2_H
#define ISDNL2_H

#include "stdafx.h"
#include "Domain/IDomain.h"

#include "iCore/MsgTimer.h"
#include "iCore/MsgObject.h"
#include "iCore/MsgThread.h"

#include "ObjLink/ObjectLink.h"
#include "ObjLink/ObjectLinksServer.h"

#include "Utils/ErrorsSubsystem.h"
#include "Utils/Random.h"

#include "isdninfr.h"
#include "isdnpack.h"
#include "packqueue.h"
#include "IsdnUtils.h"

#include "IsdnLayersInterfaces.h"
#include "tei.h"
#include "l2_const.h"
#include "L2Profiles.h"
#include "IsdnRole.h"
#include "PacketViewer.h"
#include "L2StatData.h"
#include "L2Error.h"

// TODO позакрывать интерфейсные поля

namespace ISDN 
{	
    using Domain::DomainClass;
    using iCore::MsgObject;
    using iCore::MsgThread;
    using iCore::MsgTimer;
    using ObjLink::ILinkKeyID;
    using ObjLink::ObjectLinkError;
    using ObjLink::ObjectLink;
    using boost::shared_ptr;
	
	

	//---------------------------------------------------------------

    // Layer of LAPD procedure (Recommendation Q.921)
	class IsdnL2 
        : public MsgObject,        
        public IDriverToL2,
        public IL3ToL2,
        public TeiMng,
        public ILoggable,        
        public ObjLink::IObjectLinksHost,   // server 
        public ObjLink::IObjectLinkOwner    // client
	{
        typedef ObjLink::ObjLinkStoreBinder MyBindStorage;

        enum StateByLinks
        {
            st_noWait,
            st_waitLinkConnection,
            st_downLinkDisconnected,
            st_upLinkDisconnected
        };

        struct LogRecordKinds
        {
            LogRecordKinds(ILoggable& logSession);

            iLogW::LogRecordTag objLink; // сообщения по процессу установки/разрыва ObjectLink`ов
            iLogW::LogRecordTag general; // общие сообщения
            iLogW::LogRecordTag mdlErorrs; // сообщения MdlErrorManagment
            iLogW::LogRecordTag frameErrorInfo; // ошибки пакетов
            iLogW::LogRecordTag timerEvents; // события таймеров
            iLogW::LogRecordTag warning;
            iLogW::LogRecordTag tei; // процедуры назначения tei
        };

		// old structure define
		struct L2 
		{
			L2(const shared_ptr<const L2Profile> prof, IsdnL2* own );
			~L2();
			
			FsmMachine *pl2m;
			FsmTimer t200, t203;
			int T200, N200, T203;
			IsdnL2* owner;//init in L2 constr
			TeiManage m_TeiManage;
			int tei;
			int sap;
			dword maxlen;
			
            //dword flag;
            struct
            {
                bool LAPB;
                bool LAPD;
                bool ORIG;
                bool MOD128;
                bool PEND_REL;
                bool L3_INIT;
                bool T200_RUN;
                bool ACK_PEND;
                bool REJEXC;
                bool OWN_BUSY;
                bool PEER_BUSY;
                bool DCHAN_BUSY;
                bool L1_ACTIV;
                bool ESTAB_PEND;
                bool PTP;
                bool FIXED_TEI;
                bool L2BLOCK;
            } m_flag;

            /*
            счетчики номеров пакетов по модулю 128 или 8vs 
            vs - последний отправленный
            vr - последний принятый без нарушения номерации
            va - последний подтвержденный
            sow - индекс пакета в */
			dword vs, va, vr;
			int rc;
			dword sow;
            //shared_ptr<const IsdnRole> m_pRole;
            bool m_isUserSide;
            std::vector<IsdnPacket*> m_windowAr;
			PacketQueue m_iQueue;
			PacketQueue m_uiQueue;
			int tx_cnt;


			int BCStx_cntInc(int inc);//{int ret=tx_cnt;tx_cnt+=inc;return ret;}
			void tei_l2tei ( int, void *);
            dword Random_ri();
            void SetST_L2_4();
            void SetST_L2_1();
            
            bool IsST_L2_1();
            bool IsST_L2_2();
            bool IsST_L2_3();
            bool IsST_L2_4();
            bool IsST_L2_5();
            bool IsST_L2_6();
            bool IsST_L2_7();
            bool IsST_L2_8();

            int GetMaxWindow() {return m_windowAr.size();}
            void ChangeMaxWindow(int newSize);

            ILoggable& getLogSession() {return *owner;}
            L2StatCollector& getStat() {return owner->m_stat;}
            const LogRecordKinds& getLogTags() {return owner->m_logKinds;}
            void LogPacketSend(IsdnPacket* pPack) const;
		
        private:
			
            static void DelPack(IsdnPacket* pPack){if(pPack)pPack->Delete();} // используется for_each в деструкторе	
			friend class IsdnL2;
			void DatIndOther(IsdnPacket* pPacket); // Обработка DataInd для остальных служб (not TEI)
			void DatIndTei(IsdnPacket* pPacket); // Обработка DataInd для службы TEI
			void EstabReq();
			void RelReq();
			void DatReq(IPacket *pPacket);
			void UDatReq(UPacket *pPacket);
			void MAsgnReq(int arg);
			void MRmovReq();
			void MErrResp();
			void DatInd(IsdnPacket *pPacket);
			void ActivInd();
			void DeactConf();
			void PullCnf();
			void PausInd();
			void PausConf();
		};

	public:
		
		typedef L2 Layer2;
		
        IsdnL2(DomainClass& domain, BinderToIDriver pDrvBinder,
                IsdnInfra* pInfra, const shared_ptr<const L2Profile> prof);                

        //конструктор для горячей перезагрузки
        IsdnL2(DomainClass& domain, BinderToIDriver pDrvBinder,
                BinderToIL2ToL3 l3Binder, IsdnInfra* pInfra, const shared_ptr<const L2Profile> prof);

        L2StatData getStatData() const {return m_stat.getData();}

        void ClearStat() {m_stat.Clear();}
        
		void SetUpIntf(IObjectLinksHost* pHost, IL2ToL3* pIntf);		        
		
		ObjectLink<IL2ToL3>& GetUpIntf();

		ObjectLink<IL2ToDriver>& GetDownIntf();

        BinderToIDrvToL2 GetBinderIDrvToL2();
		
        IsdnInfra *GetInfra(){return m_pInfra;}
				
		Layer2* getL2() {return &m_l2obj;}

        template<class PackType>
        PackType* CreatePacket() { return m_pInfra->CreatePacket<PackType>();}

        // Запуск процессинга внутренних таймеров
        void StartProcessing(int periodMsec = 50);

        // Останов процессинга внутренних таймеров
        void StopProcessing();

        void SetDownIntf(BinderToIDriver drvBinder);
        
        const LogRecordKinds& getLogTags()
        {
            return m_logKinds;
        }


	// Implementation IL3ToL2 (L3 receivers, processing)
    private:

		void EstablishReq();            

		void ReleaseReq();              

		void DataReq(QVector<byte> l3data); 

		void UDataReq(QVector<byte> l3data);

        void SetUpIntf(BinderToIL2ToL3 binderToL3);
		
    // Implementation TeiMng
    public:
		void MAssignReq(int arg);       

		void MRemoveReq();              

		void MErrorResponse();          
			
	// Implementation of IDriverToL2
    private:

		void DataInd(QVector<byte> packet);

		void ActivateInd();             

		void DeactivateInd();           

		void ActivateConf();            

		void DeactivateConf();          

		void PullConf();                

		void PauseInd();                

		void PauseConf();               
        

    // Implementation of IObjectLinksHost
    private:

        Utils::SafeRef<ObjLink::IObjectLinksServer> getObjectLinksServer();

    // Implementation of IDomain
    private:

        DomainClass& getDomain();

    // IObjectLinkOwner impl
    private:

        bool AllLinksIsConnected();

        void OnObjectLinkConnect(ILinkKeyID &linkID);

        void OnObjectLinkDisconnect(ILinkKeyID &linkID);

        void OnObjectLinkError(shared_ptr<ObjectLinkError> error) 
        {
        }        

	private:        

        void Process(MsgTimer*);

        void SimpleLog(const char* cstr, iLogW::LogRecordTag kind = iLogW::LogRecordTag());

		FsmMachine* CreateFsm_l2();

		void DeleteFsm_l2();

		FsmMachine* GetTeiFsm(){return m_l2obj.m_TeiManage.tei_m;}

        void GetRandomBytes(void *buf, int nbytes);

		void MErrorIndication(L2Error error);

		friend void sendToMErrorInd(IsdnL2 *pL2,char error);

    // Data Fields:
    private:

        LogRecordKinds m_logKinds;        		
		IsdnInfra* m_pInfra;
        L2StatCollector m_stat;
		Layer2 m_l2obj;
        PacketViewer m_PV;   // просмотрщик на отправке        
        DomainClass &m_domain;

		// входные интерфейсы нижнего и верхнего уровней
		ObjectLink<IL2ToDriver> m_pIntfDown;
		ObjectLink<IL2ToL3>  m_pIntfUp;	

        // state related
        StateByLinks m_linksState;        
        
        Utils::Random m_rnd;
        MsgTimer m_processingTimer; // запускает процессинг внутренних таймеров

        // link server fields:
        ObjLink::ObjectLinksServer m_server;        
        MyBindStorage m_bindStorage;
	};	
}//ISDN

#endif


