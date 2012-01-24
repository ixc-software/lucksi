#ifndef __OBJECTLINKBASIC__
#define __OBJECTLINKBASIC__

#include "stdafx.h"
#include "iCore/MsgObject.h"
#include "Utils/SafeCastPtr.h"
#include "Utils/ThreadContext.h"
#include "Domain/ObjectName.h"
#include "ObjLinkInterfaces.h"
#include "ObjectLinkMeta.h"

namespace ObjLink
{
    using boost::shared_ptr;

    // абстрактный базовый нешаблонный класс для ObjectLinkLocal и ObjectLinkRemote
    // нешаблонный класс используется для возможности определения методов в .cpp
    class ObjectLinkBasic : public iCore::MsgObject
    {
        typedef ObjectLinkBasic T;

        IObjectLinkOwnerNotify &m_owner;
        bool m_connected;
        Utils::ThreadContext m_threadContext;

        virtual void DoDisconnect() = 0;

    protected:

        void GotoConnected()
        {
            ESS_ASSERT(!Connected());
            m_connected = true;
            m_owner.OnObjectLinkConnect();
        }

        void GotoDisconnected()
        {
            ESS_ASSERT(Connected());
            m_connected = false;
            m_owner.OnObjectLinkDisconnect();
        }

        IObjectLinkOwnerNotify& Owner()
        {
            return m_owner;
        }

        void AssertThreadContext()
        {
            m_threadContext.Assert();
        }

        void SendError(ObjectLinkError *pError)
        {
            boost::shared_ptr<ObjectLinkError> error(pError);
            m_owner.OnObjectLinkError(error);
        }

        QObject* Find(const Domain::ObjectName &name);


    public:
        ObjectLinkBasic(IObjectLinkOwnerNotify &owner);

        bool Connected() const
        {
            return m_connected;
        }

        void Disconnect()
        {
            DoDisconnect();
        }

        static bool NameIsLocal(const Domain::FullObjectName &name, IObjectLinkOwner *pOwner);

    };

    // -----------------------------------------------

    // ObjectLinkBasic + интерфейс типа TIntf
    template<class TIntf>
    class ObjectLinkT : public ObjectLinkBasic
    {
        bool m_intfaceActive;
        boost::shared_ptr<TIntf> m_intf;

    protected:

        void setInterface(TIntf *pIntf)
        {
            ESS_ASSERT(m_intf.get() == 0);

            m_intf.reset(pIntf);
            m_intfaceActive = true;
        }

        void ClearInterface()
        {            
            m_intfaceActive = false;

            // этого НЕ делаем, т.к. разрушение объекта в m_intf не даст возможности
            // досылать сообщения после обрыва связи со стороны клиента
            // m_intf.reset();
        }

    public:

        ObjectLinkT(IObjectLinkOwnerNotify &owner) 
            : ObjectLinkBasic(owner), m_intfaceActive(false)
        {
        }

        TIntf* operator->()
        {
            ESS_ASSERT(m_intfaceActive);
            ESS_ASSERT(m_intf.get() != 0);
            return m_intf.get();
        }

    };

	
}  // namespace ObjLink

#endif

