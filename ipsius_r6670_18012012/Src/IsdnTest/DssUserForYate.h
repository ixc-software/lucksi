#ifndef DSSUSERFORYATE_H
#define DSSUSERFORYATE_H

#include "stdafx.h"
#include "ISDN/isdnl3.h"
#include "ISDN/DssUpIntf.h"

#include "Domain/DomainClass.h"
#include "iCore/MsgObject.h"
#include "iLog/LogWrapper.h"
#include "ObjLink/ObjectLinksServer.h"
#include "ObjLink/ObjectLinkBinder.h"

#include "Logger.h"
#include "UserCallForYate.h"

namespace IsdnTest
{
    using Domain::DomainClass;
    using iCore::MsgObject;
    using ISDN::BinderToILayerDss;
    using ISDN::ICallBackDss;
    using ISDN::DssPhoneNumber;
    using ISDN::DssCallParams;
    using ISDN::DssOutCallParams;
    using ObjLink::ObjLinkStoreBinder;
    using boost::scoped_ptr;   
      

    // Ждет выполнения/завершения входящего вызова, после создает исходящий,
    // который завершается со стороны Yate после установки соединения.
    class DssUserForYate
        : public MsgObject,        
        public ObjLink::IObjectLinksHost,   //server 
        public ObjLink::IObjectLinkOwner,   // client
        public ICallBackDss,
        public IDssCallToDssUserForYate
    {        
        Logger m_logSession;        
        scoped_ptr<UserCallForYate> m_inCall;
        scoped_ptr<UserCallForYate> m_outCall;

        // TODO только для cause - неудобно!
        IsdnInfraTest &m_infra;
        const YateL3TestProfile &m_prof;

        ObjLink::ObjectLinksServer m_server; // server
        DomainClass &m_domain;               // server        
        ObjLink::ObjectLink<ISDN::ILayerDss> m_linkToDss;
        ObjLinkStoreBinder m_myLinkBinderStorage;

    // Implementation ICallBackDss:
    private:

        void Activated() 
        {      
            m_logSession.Log("Activated");
        }

        void Deactivated() 
        {   
            m_logSession.Log("Deactivated");
        }

        void IncomingCall(IncommingCallParametrs params) 
        {         
            Writer(m_logSession).Write() << "New Incoming Call from `" 
                << params.m_callParams->GetCalling().getNumber()
                << "' to '" << params.m_callParams->GetCalled().getNumber();
            
            m_inCall.reset(new UserCallForYate(*this, params.m_bindToDssCall, params.m_callParams));
        }

        void DssErrorMsg(shared_ptr<const ISDN::DssWarning> pWarning)
        {         
            Writer(m_logSession).Write() << "Dss Warning" << pWarning->ToString();
            //m_logSession.Log("Dss Warning");
        }

    //IDssCallToDssUserForYate
    private:

        //DomainClass& getDomain() 
        const Logger& getLogSession()
        {
            return m_logSession;
        }

        IsdnInfraTest* getInfra()
        {
            return &m_infra;
        }

        const YateL3TestProfile& getProf()
        {
            return m_prof;
        }

    // IObjectLinkOwner impl
    private:

        void OnObjectLinkConnect(ILinkKeyID &linkID){}

        void OnObjectLinkDisconnect(ILinkKeyID &linkID){}

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

    public:

        DssUserForYate(DomainClass& domain, const Logger& parentLog, IsdnInfraTest &infra, const YateL3TestProfile& prof)
            : MsgObject(domain.getMsgThread()),
            m_logSession(parentLog, this),            
            m_infra(infra),
            m_prof(prof),
            m_server(domain, this),
            m_domain(domain),
            m_linkToDss(*this)
        {}

        ISDN::BinderToICallBackDss getICallBackDssBinder()
        {
            return m_myLinkBinderStorage.getBinder<ICallBackDss>(this);
        }

        void SetDownIntf(BinderToILayerDss bind)
        {
            TUT_ASSERT( bind->Connect(m_linkToDss) );
        }

        bool IsComplete()
        {
            bool inCallComplete = m_inCall.get() != 0 && m_inCall->IsComplete();
            
            // если входящий выполнен, а исходящий не создан выполняем исходящий
            if (inCallComplete && m_outCall.get() == 0)
            {
                RunOutCall();
                return false;
            }
            return inCallComplete && m_outCall->IsComplete();
        }

        void RunOutCall()
        {
            if ( !m_linkToDss.Connected() ) return;

            m_logSession.Log("***********Initiate outgoing call**************");

            // создаем параметры вызова
            shared_ptr<DssCallParams> param (DssOutCallParams::Create( 
                &m_infra, DssPhoneNumber("7190001"), DssPhoneNumber("7190000") 
            ));
            
            // создаем пользовательский объект вызова
            m_outCall.reset( new UserCallForYate(*this, param) );

            // упаковка параметров и биндера
            ISDN::ILayerDss::CreateOutCallParametrs params(  m_outCall->getBinder(), param  );
            
            m_linkToDss->CreateOutCall(params);
        }
    };


} // namespace IsdnTest

#endif
