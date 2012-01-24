#ifndef L3CALL_H
#define L3CALL_H

#include "stdafx.h"
#include "iCore/MsgObject.h"

#include "ObjLink/ObjectLink.h"
#include "ObjLink/ObjectLinksServer.h"
#include "ObjLink/ObjectLinkBinder.h"

#include "Domain/IDomain.h"

// #include "Utils/StateMachine.h"
#include "Utils/BaseSpecAllocated.h"

#include "IIsdnL3Internal.h"
#include "IL3Calls.h"
#include "IL3Call.h"
#include "L3CallFsm.h"
#include "DssUpIntf.h" 
#include "CallRef.h"
#include "DssInCallParams.h"
#include "BChannelsWrapper.h"


namespace ISDN
{    

    using boost::shared_ptr;
    using Domain::DomainClass;
    using ObjLink::ILinkKeyID;
    using ObjLink::ObjectLinkError;
    
    // Объект представления Dss вызова
    class L3Call 
        : public iCore::MsgObject,
        public ObjLink::IObjectLinkOwner, // client
        public ObjLink::IObjectLinksHost,   //server 
        public IDssCall,
        public IL3Call,
        public Utils::BaseSpecAllocated<IIsdnAlloc>,
        boost::noncopyable
    {        
        struct LogRecordKinds
        {
            LogRecordKinds(ILoggable&);

            iLogW::LogRecordTag Info;
        };

        enum State
        {
            st_noWait,
            st_waitRelByUser,       // ожидание вызванное командой от UserCall
            st_waitOnLinkConnectedForDisc34, //ждать подключения для отправки Disc cause34 и удаления
            st_inDeleting // в процессе удаления
            //st_waitRelByCallList    // ожидание вызванное командой от L3Calls(список вызовов)
        };

        // Конструктор исходящего вызова        
        L3Call(IIsdnL3Internal& IL3, BinderToUserCall link, shared_ptr<const DssCallParams> pParams, const std::string& callName);

        // конструктор входящего вызова
        L3Call(const CallRef& cref, IIsdnL3Internal& IL3);

    public:

        // Special Allocation;
        static L3Call* CreateOutCall(IIsdnL3Internal& IL3, BinderToUserCall link, shared_ptr<const DssCallParams> pParams, const std::string& callName);
        static L3Call* CreateInCall(const CallRef& cref, IIsdnL3Internal& IL3);

        ~L3Call();
        
        template<class TPacket>// TODO 
        void RoutePacket(TPacket *pPacket)
        {
            //ESS_ASSERT( m_state == st_noWait || m_state == st_waitRelByUser);
            m_DownFsm.RoutePacket(pPacket);
            //void Rout(TypeShell);
        }

        // Predicate, return true if equal
        bool CallRefEqual(const CallRef& callRef) const;

        // Выполнить процедуру срочного синхронного удаления с посылом ReleaseComplete
        void DropCall(shared_ptr<const DssCause> pCause);
        const CallRef& GetCallRef() const;
        bool HaveBChannels() const;
        const BChannelsWrapper& GetBCannels() const;
        void InitBCannels(const SharedBChansWrapper &set);
        const ILoggable& getLogSession();
        bool IsOutgoing() const 
        {
            return m_isOutgoing;
        }
        void AsyncDisconnect(shared_ptr<const DssCause> pCause);


    private:                

        // Вызывается после создания исходящего вызова и подтверждения линка
        void SendSetupReq();

        void AsyncDeleteCall();

        //void Nop(){}//временно
        
    // IL3Call impl:
    private:
        shared_ptr<const DssCallParams> GetOutCallParams();        
        void BChanBusyInSendSetup();        
        void ReleaseConfirm(); // Override

        // инициатива удаления от фсм
        void CloseCall( shared_ptr<const DssCause> pCause ); // Override
        
        BinderToDssCall GetBinder();        
        void LogCallInfo(){ LogCallInfo(""); }
        void SimpleLog(const char* msg, iLogW::LogRecordTag kind);
        const LogRecordKinds& getLogTags() const;

        // создать сообщения от имени пользователя:
        void AsyncSetupCompl();        

        // Implementation IDssCall:
        void HoldReq();
        void RetriveReq();
        void RetriveConf();
        void HoldConf();
        void ConnectRsp();
        void SetLink(BinderToUserCall pL4);
        void Disconnent(  shared_ptr<const DssCause> pCause );        
        void Alerting();

        void AddressComplete();
        void AddressIncomplete();

        void LogCallInfo(const std::string& userCallName);

    // обработчики внутренних асинхронных команд к fsm
    private:
        void SetupComplete();// нет в интерфейсе, дсс сам шлет команду от имени пользователя
        
    // IObjectLinkOwner impl
    private:

        void OnObjectLinkConnect(ILinkKeyID &linkID);

        void OnObjectLinkDisconnect(ILinkKeyID &linkID);

        void OnObjectLinkError(shared_ptr<ObjectLinkError> error) 
        {}

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
               
        shared_ptr<const DssInCallParams> m_pInParams;
        shared_ptr<const DssCallParams> m_pOutParams;
        bool m_isOutgoing;        
        SharedBChansWrapper m_setBChans;
        const CallRef m_callRef;            
        ILoggable m_logSession;
        LogRecordKinds m_logTags;
        L3CallFsm m_DownFsm;
        IL3Calls* m_pCalls;
        IIsdnL3Internal& m_IL3;        
        State m_state;

        DomainClass &m_domain;
        ObjLink::ObjectLinksServer m_server;             
        ObjLink::ObjectLink<IDssCallEvents> m_linkToUserCall;        

        ObjLink::ObjLinkStoreBinder m_myLinkStorage;

        //std::string m_InDeleting;//временная переменная TODO убрать!
    };

} // namespace ISDN

#endif

