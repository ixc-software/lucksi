#ifndef DSSUSERCALL_H
#define DSSUSERCALL_H

//#include "iCore/MsgTimer.h"
#include "Domain/IDomain.h"

#include "ObjLink/ObjectLink.h"
#include "ObjLink/ObjectLinksServer.h"
#include "ObjLink/ObjectLinkBinder.h"

#include "Utils/IBasicInterface.h"
#include "ISDN/DssUpIntf.h"
#include "IDssScenarioBase.h"
#include "logger.h"
#include "infratest.h"

namespace IsdnTest
{
  
    using iCore::MsgThread; //?

    using ObjLink::ILinkKeyID;
    using ObjLink::ObjectLinkError;    
    using ISDN::BinderToUserCall;
    using ISDN::BinderToDssCall;
    using ISDN::IDssCall;
    using boost::shared_ptr;
    using ISDN::SetBCannels;
    using ISDN::CallWarning;
    using ISDN::DssCause;
    using ISDN::DssCallParams;
    using ISDN::DssPhoneNumber;
    using ISDN::IsdnStack;

    class OutgoingDssUserCall;
    class IncommingDssUserCall;


    // интерфейс внутренних Фсм реализованных в конкретных вызововах-подсценариях
    class IFsmIncomming : public Utils::IBasicInterface
    {
    public:

        // обработчики сообщений от DssCall        
        virtual void OnConnectConfirm() = 0;
        virtual void OnDssCallErrorMsg( shared_ptr<const CallWarning> ) = 0;
        virtual void OnDisconnent( shared_ptr<const DssCause> ) = 0;

        virtual void OnIncommingCall(BinderToDssCall dssCall, shared_ptr<const DssCallParams> pParams) = 0;

        virtual void OnLinkDisc() = 0;
        virtual void OnLinkConnect() = 0;

        // Начало работы Fsm
        virtual void OnRun() = 0;

        // Проверка завершения
        virtual bool StateIsComplete() = 0;

        // Противоположная сторона внезапно порвала связь с такой причиной
        virtual void OpposideDropped(shared_ptr<const DssCause> cause)
        {
            ESS_ASSERT(0 && "Drop procedure under a ban!");
        }

        // Внезапно порвать связь
        virtual void Drop(shared_ptr<const DssCause> cause, OutgoingDssUserCall& opposide)
        {
            ESS_ASSERT(0 && "Drop procedure under a ban!");
        }
    };

    class IFsmOutgoing : public Utils::IBasicInterface
    {
    public:

        // обработчики сообщений от DssCall 
        virtual void OnAlerting() = 0;
        virtual void OnConnected() = 0;
        virtual void OnConnectConfirm() = 0;
        virtual void OnDssCallCreated(ISDN::IDssCallEvents::DssCallCreatedParametrs) = 0;
        virtual void OnDssCallErrorMsg( shared_ptr<const CallWarning> ) = 0;
        virtual void OnDisconnent( shared_ptr<const DssCause> ) = 0;
        
        virtual void OnLinkDisc() = 0;
        virtual void OnLinkConnect() = 0;

        // Начало работы Fsm
        virtual void OnRun() = 0;

        // Проверка завершения
        virtual bool StateIsComplete() = 0;

        // Противоположная сторона внезапно порвала связь с такой причиной
        virtual void OpposideDropped(shared_ptr<const DssCause> cause)
        {
            ESS_ASSERT(0 && "Drop procedure under a ban!");
        }

        // Внезапно порвать связь
        virtual void Drop(shared_ptr<const DssCause> cause, IncommingDssUserCall& opposide)
        {
            ESS_ASSERT(0 && "Drop procedure under a ban!");
        }
    };
    
    // интерфейс сабсценария предоставляемый вызову
    class ISubsToUserCall : public Utils::IBasicInterface
    {
    public:
        virtual DssPhoneNumber& getNumReceiver() = 0;
        virtual const Logger& getLogger() = 0;
        virtual IsdnInfraTest& getInfra() = 0;
        virtual void DeleteOutCall() = 0;
        virtual void DeleteInCall() = 0;

        virtual void AsyncDeleteOutCall() = 0;
        virtual void AsyncDeleteInCall() = 0;

        virtual IDssScenarioBase& GetIScenario() = 0;
    };
   

    // базовый класс для конкретных вызовов-подсценариев(составная часть подсценариев)
    class DssUserCall
        : public iCore::MsgObject,
        public ISDN::IDssCallEvents,        
        public ObjLink::IObjectLinkOwner, // client
        public ObjLink::IObjectLinksHost,   //server 
        boost::noncopyable
    {
        IDssScenarioBase& m_owner;
        ISubsToUserCall& m_subs;
        IsdnStack* m_pStack;
        //MsgTimer m_dropTimer;

        Domain::DomainClass &m_domain;
        ObjLink::ObjectLinksServer m_server;             
        ObjLink::ObjectLink<ISDN::IDssCall> m_link;
        ObjLink::ObjLinkStoreBinder m_myLinkStorage;
    
    protected:

        // inherit types definition
        struct DssCallCreatedParams
        {
            BinderToDssCall m_link;
            shared_ptr<const SetBCannels> m_bChannels;
            
        };       

        typedef ObjLink::ObjectLink<ISDN::IDssCall> Link;        

    protected:

        DssUserCall(ISubsToUserCall& subs)
            : MsgObject( subs.GetIScenario().GetIDomain().getDomain().getMsgThread() ),
            m_owner(subs.GetIScenario() ),
            m_subs(subs),
            m_domain( m_owner.GetIDomain().getDomain()),
            m_server(m_domain, this), //последовательность!
            m_link(*this),            
            m_pStack(0)
            //m_dropTimer(this, &DssUserCall::OnDropTimer),            
        {}

        ISubsToUserCall& getSubs()
        {
            return m_subs;
        }

        Link& getLinkToDssCall()
        {
            return m_link;
        }

        BinderToUserCall getMyBinder()
        {
            return m_myLinkStorage.getBinder<ISDN::IDssCallEvents>(this);
        }

        virtual void DeleteThisCall() = 0;
        virtual void AsyncDeleteThisCall() = 0;

        IsdnStack& getStack()
        {
            ESS_ASSERT(m_pStack);
            return *m_pStack;
        }
        
    // IObjectLinkOwner impl
    private:
            

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

        Domain::DomainClass& getDomain()
        {
            return m_domain;
        }        
          

    // implementation IDssCallEvents
    private:

        //void xxx()
        //{
        //    PutMsg(this, &DssUserCall::OnXxx);
        //}

        //virtual void OnConnectConfirm() = 0;
        void MoreInfo( Info info ) 
        {
            ESS_HALT("todo. no overlap tests");
        }

        void ProgressInd(ISDN::IeConstants::ProgressDscr descr)
        {
            ESS_HALT("Unexpected in current tests.");
        }

    private:

        virtual void OnRun() = 0;

    public:        

        // Асинхронная команда запуска
        void Run()
        {            
            PutMsg(this, &DssUserCall::OnRun);
        }

        void SetStack(IsdnStack& stack)
        {
            m_pStack = &stack;
        }

        ~DssUserCall(){}

        
    };

    //--------------------------------------------------------------------------------------------

    // базовый класс для конкретных входящих вызовов-подсценариев(составная часть подсценариев)
    // перенаправление команд в имлементацию внутренней fsm вызова-подсценария
    class IncommingDssUserCall
        : public DssUserCall
    {
        IFsmIncomming* m_pFsm;
        //Logger* m_pLog; // TODO возможно здесь не нужен
        DssPhoneNumber m_numReceiver;

        void HoldInd()
        {
            ESS_ASSERT(0 && "Not implemented");
        }

        void RetriveInd()
        {
            ESS_ASSERT(0 && "Not implemented");
        }
        
        void DssCallCreated( DssCallCreatedParametrs )
        {
            TUT_ASSERT(0 && "DssCallCreated called on incoming call");
        }

        void Alerting()
        {
            TUT_ASSERT(0 && "Alerting called on incoming call");
        }

        void Connected()
        {
            TUT_ASSERT(0 && "Connected called on incoming call");
        }

        // redirection messages to fsm:
        void ConnectConfirm()
        {
            Assert(); m_pFsm->OnConnectConfirm(); 
        }

        void DssCallErrorMsg( shared_ptr<const CallWarning> e)
        {
            Assert(); m_pFsm->OnDssCallErrorMsg(e);
        }
        
        void Disconnent( shared_ptr<const DssCause> cause)
        {
            Assert(); m_pFsm->OnDisconnent(cause);
        }
  
        
        void OnObjectLinkDisconnect(ILinkKeyID &linkID) //override link events
        {
            Assert(); m_pFsm->OnLinkDisc();
        }

        void OnObjectLinkConnect(ILinkKeyID &linkID) //override link events
        {
            Assert(); m_pFsm->OnLinkConnect();
        }

        void OnRun() // override DssUserCall
        {
            Assert(); m_pFsm->OnRun();
        }        

    //own methods:
    private:           

        void Assert()
        {
            ESS_ASSERT(m_pFsm); // ошибка тестирующей логики
        }       

    protected:

        void SetFsm(IFsmIncomming* pFsm)
        {
            m_pFsm = pFsm;
        }        
           
        IncommingDssUserCall(ISubsToUserCall& subs)
            : DssUserCall(subs), m_pFsm(0)//, m_pLog(subs.getLogger() )
        {}

        void DeleteThisCall()
        {
            getSubs().DeleteInCall();            
        }

        void AsyncDeleteThisCall()
        {
            getSubs().AsyncDeleteInCall();
        }

    public:

        bool IsComplete()
        {
            Assert();
            return m_pFsm->StateIsComplete();
        }

        void OpposideDropped(shared_ptr<const DssCause> cause)
        {
            m_pFsm->OpposideDropped(cause);
        }

        void OnIncomingCall( BinderToDssCall dssCall, shared_ptr<const DssCallParams> pParams )
        {
            Assert();
            m_pFsm->OnIncommingCall(dssCall, pParams);
        }     

        void Drop(shared_ptr<const DssCause> cause, OutgoingDssUserCall& opposide)
        {            
            m_pFsm->Drop(cause, opposide);
        }
        
    };

    //--------------------------------------------------------------------------------------------

    // базовый класс для конкретных исходящих вызовов-подсценариев(составная часть подсценариев)
    // перенаправление команд в имлементацию внутренней fsm вызова-подсценария
    class OutgoingDssUserCall
        : public DssUserCall
    {
        IFsmOutgoing* m_pFsm;
        //Logger* m_pLog;// TODO возможно здесь не нужен

        void HoldInd()
        {
            ESS_ASSERT(0 && "Not implemented");
        }

        void RetriveInd()
        {
            ESS_ASSERT(0 && "Not implemented");
        }

        void ConnectConfirm()
        {
            TUT_ASSERT(0 && "ConnectConfirm called on outgoing call");
            //m_pFsm->OnConnectConfirm();
        }

        // redirection messages to fsm:
        void Alerting() 
        { 
            Assert(); m_pFsm->OnAlerting(); 
        }

        void Connected()
        {
            Assert(); m_pFsm->OnConnected();
        }

        void DssCallCreated(ISDN::IDssCallEvents::DssCallCreatedParametrs param)
        {
            Assert(); m_pFsm->OnDssCallCreated(param);
        }

        void DssCallErrorMsg( shared_ptr<const CallWarning> e) 
        {
            Assert(); m_pFsm->OnDssCallErrorMsg(e);
        }
        
        void Disconnent( shared_ptr<const DssCause> cause) 
        {
            Assert(); m_pFsm->OnDisconnent(cause);
        }

        void OnObjectLinkDisconnect(ILinkKeyID &linkID) //override link events
        {
            Assert(); m_pFsm->OnLinkDisc();
        }

        void OnObjectLinkConnect(ILinkKeyID &linkID) //override link events
        {
            Assert(); m_pFsm->OnLinkConnect();
        }

        void OnRun()  // override DssUserCall
        {
            Assert(); m_pFsm->OnRun();
        }                

        void Assert()
        {
            ESS_ASSERT(m_pFsm);
        }

    protected:

        void DeleteThisCall()
        {
            getSubs().DeleteOutCall();            
        }

        void AsyncDeleteThisCall()
        {
            getSubs().AsyncDeleteOutCall();
        }

        void SetFsm(IFsmOutgoing* pFsm)
        {
            m_pFsm = pFsm;
        }       

        OutgoingDssUserCall(ISubsToUserCall& subs)
            : DssUserCall(subs), m_pFsm(0)//, m_pLog( subs.getLogger() )
        {}

    public:

        bool IsComplete()
        {
            Assert();
            return m_pFsm->StateIsComplete();
        }

        void OpposideDropped(shared_ptr<const DssCause> cause)
        {
            m_pFsm->OpposideDropped(cause);
        }

        void Drop(shared_ptr<const DssCause> cause, IncommingDssUserCall& opposide)
        {           
            m_pFsm->Drop(cause, opposide);
        }
    };

   
} // namespace IsdnTest

#endif

