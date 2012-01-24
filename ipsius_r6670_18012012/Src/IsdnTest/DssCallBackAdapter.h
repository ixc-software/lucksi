#ifndef DSSCALLBACKADAPTER_H
#define DSSCALLBACKADAPTER_H

#include "stdafx.h"
#include "ISDN/DssUpIntf.h"
#include "Domain/DomainClass.h"
#include "IncommingCallRouter.h"
#include "Utils/IBasicInterface.h"


namespace IsdnTest
{
    using namespace ISDN;
    using boost::shared_ptr;
    using Domain::DomainClass;

    //Интерфейс внутренней fsm сценария
    class IScnFsm : Utils::IBasicInterface
    {
    public:
        virtual void Activated(IsdnStack* from) = 0;
        virtual void Deactivated(IsdnStack* from) = 0;
        
        virtual void DssErrorMsg(shared_ptr<const DssWarning> pWarning, IsdnStack* from) = 0;
        virtual void RestartComplete(bool ok, const  std::string& info, IsdnStack* from) = 0;
    };

    // Сливает сообщения полученные по ICallbackDss владельцу 
    // или сценарию, указывая один из двух стеков откуда оно было полученно
    class DssCallBackAdapter 
        : public iCore::MsgObject,        
        public ObjLink::IObjectLinksHost,   //server 
        public ICallbackDss,
        boost::noncopyable
    {        
        IScnFsm& m_owner;
        IncommingCallRouter& m_inCallRouter;
        IsdnStack* m_pStack;

        ObjLink::ObjectLinksServer m_server;//server
        DomainClass &m_domain;     //server        
        ObjLink::ObjLinkStoreBinder m_myLinkBinderStorage;

    // Implementation ICallbackDss:
    private:

        void Activated() 
        { 
            ESS_ASSERT(m_pStack);
            m_owner.Activated(m_pStack); 
        }
        
        void Deactivated() 
        {
            ESS_ASSERT(m_pStack);
            m_owner.Deactivated(m_pStack); 
        }

        void IncomingCall(IncommingCallParametrs params) 
        {
            ESS_ASSERT(m_pStack);
            m_inCallRouter.IncomingCall(params.m_bindToDssCall, params.m_callParams, m_pStack);
        }

        void DssErrorMsg(shared_ptr<const DssWarning> pWarning)
        {
            ESS_ASSERT(m_pStack);
            m_owner.DssErrorMsg(pWarning, m_pStack);
        } 

        void RestartComplete(CompleteInfo info)
        {
            ESS_ASSERT(m_pStack);
            m_owner.RestartComplete(info.m_ok, info.m_addInfo, m_pStack);
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

    public:

        DssCallBackAdapter(DomainClass &domain, IScnFsm* pOwner, IncommingCallRouter& inCallRouter)
            : MsgObject(domain.getMsgThread()),
            m_owner(*pOwner),
            m_inCallRouter(inCallRouter),
            m_pStack(0),
            m_server( domain, this ),
            m_domain( domain )
        {
            ESS_ASSERT(pOwner);
        }

        ~DssCallBackAdapter()
        {
            m_owner.Deactivated(m_pStack);// тут должно быть сообщение
        }

        void SetStack(IsdnStack* pStack)
        {
            m_pStack = pStack;
        }

        shared_ptr<ObjLinkBinder<ICallbackDss> > GetBinder()
        {
            return m_myLinkBinderStorage.getBinder<ICallbackDss>(this);
        }
    };
} // IsdnTest

#endif

