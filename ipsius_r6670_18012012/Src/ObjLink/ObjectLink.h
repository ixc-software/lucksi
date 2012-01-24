#ifndef __OBJECTLINK__
#define __OBJECTLINK__

#include "stdafx.h"
#include "iCore/MsgObject.h"
#include "iCore/MsgThread.h"
#include "ObjectLinkLocal.h"
// #include "Domain/IDomain.h"
#include "Domain/DomainClass.h"

namespace ObjLink
{

    // end-user класс, позвол€ющий локально или удаленно использовать интерфейс TIntf
    // TIntf must be IObjectLinkInterface
    template<class TIntf>
    class ObjectLink : 
        public iCore::MsgObject,
        public IObjectLinkOwnerNotify
    {
        typedef ObjectLink<TIntf> T;

        IObjectLinkOwner &m_owner;
        boost::shared_ptr< ObjectLinkT<TIntf> > m_link;

        void AssertLinkNotEmpty() const
        {
            ESS_ASSERT(m_link.get() != 0);
        }

        ILinkKeyID& KeyID() { return *this; }
        const ILinkKeyID &KeyID() const { return *this; }
        void SendObjectLinkConnect()
        {
            m_owner.OnObjectLinkConnect(KeyID());
        }

        void SendObjectLinkDisconnect()
        {
            m_owner.OnObjectLinkDisconnect(KeyID());
        }

        void SendObjectLinkError(boost::shared_ptr<ObjectLinkError> error)
        {
            m_owner.OnObjectLinkError(error);
        }


    /* IObjectLinkOwnerNotify impl
       реализуем дл€ того, чтобы все событи€ клиенту (интерфейс IObjectLinkOwner) заворачивались 
       именно через этот объект, а не через объект в поле m_link, т.к. как его 
       врем€ жизни короче чем врем€ жизни ObjectLink<TIntf>
    */
    private:

        void OnObjectLinkConnect() 
        { 
            PutMsg(this, &T::SendObjectLinkConnect); 
        }

        void OnObjectLinkDisconnect()
        {
            PutMsg(this, &T::SendObjectLinkDisconnect); 
        }

        void OnObjectLinkError(boost::shared_ptr<ObjectLinkError> error)
        {
            PutMsg(this, &T::SendObjectLinkError, error);
        }

    // IDomain impl
    private:

        Domain::DomainClass& getDomain()
        {
            return m_owner.getDomain();
        }

    public:

        ObjectLink(IObjectLinkOwner &owner) 
            : iCore::MsgObject(owner.getDomain().getMsgThread()), 
            m_owner(owner)
        {
            // ...
        }

        void Connect(const Domain::FullObjectName &name)
        {
            ESS_ASSERT(m_link.get() == 0);
            
            bool local = ObjectLinkBasic::NameIsLocal(name, &m_owner);

            if (local)
            {
                ObjectLinkLocal<TIntf> *pLink = 
                    new ObjectLinkLocal<TIntf>(m_owner);
                m_link.reset(pLink);
                pLink->Connect(name);                
            }
            else
            {
                ESS_ASSERT(0 && "Remote links unsupported!");
            }
        }

        // TServer must be TIntf, IObjectLinksHost
        template<class TServer>
        void Connect(TServer *pServer)
        {
            Connect(pServer, pServer);
        }

        // local link
        void Connect(IObjectLinksHost *pHost, TIntf *pIntf)
        {
            if (m_link.get() != 0)
            {
                ESS_ASSERT(!m_link->Connected());
            }
            
            ObjectLinkLocal<TIntf> *pLink = 
                new ObjectLinkLocal<TIntf>(KeyID());
            m_link.reset(pLink);
            pLink->Connect(pHost, pIntf);
        }


        void Disconnect()
        {
            AssertLinkNotEmpty();

            m_link->Disconnect();
        }

        // можно вызывать только после того, как был вызван Connect()
        bool GoesDisconnected() const
        {
            AssertLinkNotEmpty();
            return m_link->Connected();
        }

        bool Connected() const
        {
            if (m_link.get() == 0) return false;
            return GoesDisconnected();
        }

        TIntf* operator->()
        {
            AssertLinkNotEmpty();

            ObjectLinkT<TIntf> *p = m_link.get();            
            return p->operator->();
        }

        bool Equal(ILinkKeyID &keyID) const
        {
            return ((&KeyID()) == (&keyID));
        }


    };

}  // namespace ObjLink

#endif





