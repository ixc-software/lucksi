#ifndef USERCALLFORYATE_H
#define USERCALLFORYATE_H

#include "Domain/DomainClass.h"
#include "iCore/MsgObject.h"
#include "iCore/MsgTimer.h"
#include "iLog/LogWrapper.h"
#include "ObjLink/ObjectLinksServer.h"
#include "ObjLink/ObjectLinkBinder.h"

#include "ISDN/DssUpIntf.h"

#include "Logger.h"
#include "InfraTest.h"

namespace IsdnTest
{

    using boost::shared_ptr;
    using Domain::DomainClass;
    using iCore::MsgObject;
    using ObjLink::ILinkKeyID;
    using ObjLink::ObjectLinkError;    
    using ISDN::IDssCallEvents;
    using ISDN::IDssCall;
    using ISDN::BinderToDssCall;
    using ISDN::DssCallParams;
    
    using ObjLink::ObjLinkStoreBinder;

    //class IsdnInfraTest;
    class YateL3TestProfile;

    class IDssCallToDssUserForYate
    {
    public:
        virtual DomainClass& getDomain() = 0;
        virtual const Logger& getLogSession() = 0; 
        virtual IsdnInfraTest* getInfra() = 0;
        virtual const YateL3TestProfile& getProf() = 0;
    };

   
    // Пользовательский объект вызова для Yate - теста.
    // Имплиментирует интерфес вызова, транслирует обращения в события и 
    // направляет в FsmIncoming или FsmOutgoing
    class UserCallForYate
        : public MsgObject,
        public ObjLink::IObjectLinkOwner, // client
        public ObjLink::IObjectLinksHost,   //server 
        public IDssCallEvents
    {
        class FsmIncoming;
        class FsmOutgoing;     

        // Internal Fsm Events:
        struct EvAlerting{};
        struct EvConnectCnf{};
        struct EvConnected{};
        typedef DssCallCreatedParametrs EvCallCreated;
        typedef boost::shared_ptr<const ISDN::CallWarning> Warning;
        typedef Warning EvWarning;
        typedef boost::shared_ptr<const ISDN::DssCause> Cause;
        typedef Cause EvDisc;       
        struct EvLinkConnect{};
        struct EvLinkDisc{};
        struct EvDiscTimer{};
        
        IDssCallToDssUserForYate& m_owner;
        Logger m_logSession;
        iCore::MsgTimer m_tDisc; 

        boost::scoped_ptr<FsmIncoming> m_pInFsm;
        boost::scoped_ptr<FsmOutgoing> m_pOutFsm;

        ObjLink::ObjectLinksServer m_server;//server
        DomainClass &m_domain;     //server        

        ObjLink::ObjectLink<IDssCall> m_link;

        ObjLinkStoreBinder m_myLinkBinderStorage;

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

    // IDssCallEvents Impl
    private:

        void HoldInd(){}
        void RetriveInd() {}

        void Alerting();        

        void ConnectConfirm();

        void Connected();
        
        void DssCallCreated(DssCallCreatedParametrs params);

        void DssCallErrorMsg(Warning pWarning);
        
        void Disconnent(Cause pCause);

    private:

        void SendLink()
        {
            m_link->SetLink(m_myLinkBinderStorage.getBinder<IDssCallEvents>(this));
        }

        void OnDiscTimer(iCore::MsgTimer*);

        template<class TEvent>
        void EventToFsm(const TEvent& ev);
        
    public:

        // Incoming call
        UserCallForYate(IDssCallToDssUserForYate& owner, 
            BinderToDssCall bindToDssCall, shared_ptr<const DssCallParams> callParams);

        // Outgoing call
        UserCallForYate(IDssCallToDssUserForYate& owner, shared_ptr<const DssCallParams> callParams);

        bool IsComplete();

        ISDN::BinderToUserCall getBinder()
        {
            return m_myLinkBinderStorage.getBinder<IDssCallEvents>(this);
        }
    };

    
} // namespace IsdnTest

#endif
