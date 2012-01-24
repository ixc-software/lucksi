#ifndef ISDNL3_H
#define ISDNL3_H

#include "stdafx.h"

#include "iCore/MsgThread.h" 
#include "iCore/MsgObject.h"

#include "Domain/IDomain.h"
#include "ObjLink/ObjectLinkBinder.h"
#include "ObjLink/ObjectLinksServer.h"

#include "L3Profiles.h" 
#include "isdninfr.h"

#include "IsdnLayersInterfaces.h"
#include "IIsdnLog.h"

#include "L3Calls.h"

#include "L3PacketFactory.h"
#include "IeFactory.h"

#include "DssUpIntf.h"
#include "IIsdnL3Internal.h"
#include "CallRefGenerator.h"
#include "BChannelsDistributor.h"
#include "DssTimer.h"
#include "L3StatData.h"

namespace ISDN
{
    using boost::shared_ptr;
    using Domain::IDomain;       
    using Domain::DomainClass;

    class IPacket; 
    class UPacket; 

    using ObjLink::ObjectLink;
    using ObjLink::ObjLinkBinder;

    class PacketRestart;
    class PacketRestartAck;
    class PacketStatus;

    // Dss-layer of ISDN system
    class IsdnL3 
        : public iCore::MsgObject,
        public ObjLink::IObjectLinkOwner, // client
        public ObjLink::IObjectLinksHost,   //server 
        public ILayerDss,
        public IL2ToL3,
        public IIsdnL3Internal,
        public ILoggable,        
        boost::noncopyable
    {
        typedef IsdnL3 L3;

        enum StateByLinks
        {
            st_noWait,
            st_waitLinkConnection,
            st_downLinkDisconnected,
            st_upLinkDisconnected
        };

        enum DssState
        {
            st_deactivated,
            st_goesActivate,
            st_restartReq,
            st_activated,            
            st_goesDeactivate
        };

        struct LogRecordKinds
        {
            LogRecordKinds(ILoggable&);

            iLogW::LogRecordTag ClientMsg;
            iLogW::LogRecordTag FromL2Msg;
            iLogW::LogRecordTag ToL2Msg;
            iLogW::LogRecordTag ObjLinkMsg;
            iLogW::LogRecordTag WarningMsg;
            iLogW::LogRecordTag Error;
            iLogW::LogRecordTag TimerEvents;
            iLogW::LogRecordTag ExpandedInfo;
            iLogW::LogRecordTag OwnAction;
            iLogW::LogRecordTag GeneralInfo;
            iLogW::LogRecordTag ChangeDssState;
        };

    public:

        IsdnL3(IDomain& iDomain, IsdnInfra& infr, const L3Profile &profile);

        // конструктор используемый при перезагрузке
        IsdnL3(IDomain& iDomain, IsdnInfra& infr, const L3Profile &profile,
            BinderToICallBackDss pMng, IObjectLinksHost* pL2Host, IL3ToL2* pL2Intf);

        ~IsdnL3();
        
        void SetMngLink(BinderToICallBackDss pMng);

        void SetL2Link (IObjectLinksHost* pHost, IL3ToL2* pIntf);        
        
        template<class TIntf>
        shared_ptr<ObjLinkBinder<TIntf> > GetBinder()
        {
            return m_myLinkBinderStorge.getBinder<TIntf>(this);
        }

        IeConstants::Location GetLocation();        

        L3StatData getStatData() const {return m_stat.getData();}

        void ClearStat();

        void ProcessGlobCrefPack(const PacketRestart*);
        void ProcessGlobCrefPack(const PacketRestartAck*);
        void ProcessGlobCrefPack(const PacketStatus*);      

        void UpdateTraceOptions(const DssTraceOption& option);

    private:        

        struct EvT309;
        void ExpireT309(const EvT309&);

        struct EvT316;
        void ExpireT316(const EvT316&);

        struct EvTReActivateReq;
        void ExpireTReActivateReq(const EvTReActivateReq&);

        struct EvWaitFreeWindFailed;
        void ExpireTWaitFreeWinInd(const EvWaitFreeWindFailed&);

        struct EvFErrPeriod;
        void ExpireWaitFErr(const EvFErrPeriod&);

        // links state assertion for methods implementation
        void StateAssertForMngIntf();
        void StateAssertForL2Intf();

        bool AllLinksConnected()
        {
            return m_pL2.Connected() && m_pMng.Connected();
        }   

        void SimpleLog(const char* msg, iLogW::LogRecordTag kind)
        {
            if (getTraceOn())
                DoLog(msg,kind);
        }

        bool TraceIeList()const;
        bool TraceIeContent()const;

        void AllCallsClearNotification();
        void ChangeDssState(DssState newState);
    
        void Warning(shared_ptr<const DssWarning> warning);
        void TraceDumpforIncomming(QVector<byte> data); 

        void RestartAllIntf();

        void DoActivation();
        

    // IL2ToL3 implementation
    private:
        
        void EstablishInd();

        void EstablishConf();

        void ReleaseConf();

        void ReleaseInd();

        void DataInd(QVector<byte>);        

        void UDataInd(QVector<byte>);

        void SetDownIntf(BinderToIL3ToL2 binderToL2);

        void FreeWinInd();

        void ErrorInd(L2Error error);
        
    // ILayerDss implementation
    private:
        void CreateOutCall( CreateOutCallParametrs params);                
        void Activate();        
        void Deactivate();
        
        void RestartReq();
        
    // IIsdnL3Internal implementation
    private:

        IL3Calls* GetIL3Calls();
        ObjLink::ObjectLink<ICallbackDss>& GetIDssMng();        
        void Send(const L3Packet& l3Pack);
        CallRefGenerator& GetCallRefGen();
        BChannelsDistributor& GetBChanDistributor();
        iCore::MsgThread& GetThread(); // TODO? IDomain?
        Domain::IDomain& GetIDomain();
        IsdnInfra& GetInfra();
        const DssTraceOption& getTraceOption()const{return m_profile.getTraceOption();}
        const ILoggable& getParentSession(){return *this;}
        //const IsdnRole& GetRole() const;
        bool IsUserSide() const;
        const L3Profile::Options& GetOptions()const;
        const DssTimersProf& GetTimersProf()const;
        void SendStackWarning(shared_ptr<const StackWarning>);
        IeFactory& getIeFactory() { return m_ieFactory; }
        L3StatCollector& getStat() { return m_stat; }
        

    // IObjectLinkOwner impl
    private:

        void OnObjectLinkConnect(ObjLink::ILinkKeyID &linkID);

        void OnObjectLinkDisconnect(ObjLink::ILinkKeyID &linkID);

        void OnObjectLinkError(shared_ptr<ObjLink::ObjectLinkError> error);

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
    
    private:
        L3Profile m_profile;

        LogRecordKinds m_logTags;
        L3StatCollector m_stat;
        DomainClass &m_domain;        
        ObjectLink<IL3ToL2> m_pL2;          //as client
        ObjectLink<ICallbackDss> m_pMng;    //as client 
        StateByLinks m_stateByLinks;

        iCore::MsgThread& m_thread; // use for L3Call 
        IDomain& m_iDomain;
        IsdnInfra& m_infra;                
        DssState m_dssState;
        
        IeFactory m_ieFactory;
        L3PacketFactory m_packFactory;        
        CallRefGenerator m_callRefGen;                                          

        BChannelsDistributor m_distributor;
        
        StopDssTimersGroup StopAllTimers;
        struct EvT309{ IDssTimer* pTimer; EvT309(IDssTimer* p) : pTimer(p){} };
        DssTimer<L3, EvT309> T309;
        struct EvT316{ EvT316(IDssTimer*){} };
        DssTimer<L3, EvT316> T316;
        struct EvTReActivateReq{ IDssTimer* pTimer; EvTReActivateReq(IDssTimer* p) : pTimer(p){} };
        DssTimer<L3, EvTReActivateReq> m_timerReActivateReqL3;
        struct EvWaitFreeWindFailed{ EvWaitFreeWindFailed(IDssTimer*){} };
        DssTimer<L3, EvWaitFreeWindFailed> m_timerWaitFreeWinInd;        
        
        struct EvFErrPeriod{ EvFErrPeriod(IDssTimer* p){} };
        DssTimer<L3, EvFErrPeriod> m_timerWaitFErr;
        int m_FCounter;

        ObjLink::ObjectLinksServer m_server;//server        
        bool m_recreated;
        L3Calls m_calls;
        ObjLink::ObjLinkStoreBinder m_myLinkBinderStorge;          
    };

} // namespace ISDN

#endif

