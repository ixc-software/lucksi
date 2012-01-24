#ifndef __OBJECTLINKLOCAL__
#define __OBJECTLINKLOCAL__

#include "stdafx.h"
#include "Platform/Platform.h"
#include "ObjectLinkBasic.h"

namespace ObjLink
{
	
    // локальный ObjectLink
    template<class TIntf>
    class ObjectLinkLocal : 
        public ObjectLinkT<TIntf>,
        public IServerToObjectLink,
        public ILocalProxyHost
    {
        typedef ObjectLinkLocal<TIntf> T;
        typedef ObjectLinkT<TIntf> Base;

        SafeRef<IObjectLinksServer> m_server;
        TIntf *m_pDstIntf;

        SafeRef<IServerToObjectLink> ServerID() 
        {
            return SafeRef<IServerToObjectLink>(this);
        }

        void ClearServersideData(bool clearIntf)
        {
            if (m_server.IsEmpty()) return;

            m_server.Clear();
            if (clearIntf) m_pDstIntf = 0;
        }

        // этот метод вызываетс€ только если обрыв св€зи происходит со стороны клиента
        void DoDisconnect()  // override
        {
            Base::ClearInterface();  // пользоватьс€ интерфейсом после обрыва его клиентом нельз€

            if (m_server.IsEmpty()) return;

            SafeRef<IServerToObjectLink> link(this);
            m_server->ObjectLinkDisconnect(link);

            ClearServersideData(true);

            Base::GotoDisconnected();
        }

        std::string InterfaceName()
        {
            return Platform::FormatTypeidName(typeid(TIntf).name());
        }


    // IServerToObjectLink impl
    private:

        void ObjectLinkConnectConf() 
        {
            Base::AssertThreadContext();
            Base::GotoConnected();
        }

        // обрыв св€зи со стороны сервера, Ќ≈ должен вызывать DoDisconnect()
        // интерфейсом можно пользоватьс€, но сообщени€ доходить не будут
        void ObjectLinkDisconnect() 
        {
            Base::AssertThreadContext();
            ClearServersideData(true);
            Base::GotoDisconnected();
        }

    // ILocalProxyHost impl
    private:

        iCore::MsgThread& getMsgThreadForProxy()
        {
            return Base::getMsgThread();
        }

        Utils::IBasicInterface* getDestinationInterfaceForProxy()
        {            
            return m_pDstIntf;
        }

        bool ConnectedForProxy()
        {
            bool connected = Base::Connected(); // (m_pDstIntf != 0); 

            if (!connected) Base::SendError( new ErrorLocalCallLost(&Base::Owner()) );

            return connected; 
        }

        void SendMsgToDestinationObject(iCore::MsgBase *pMsg)
        {
            SafeRef<IServerToObjectLink> link(this);
            m_server->ObjectLinkSendMsg(link, pMsg);
        }

    public:

        ObjectLinkLocal(IObjectLinkOwnerNotify &owner) : 
          ObjectLinkT<TIntf>(owner), m_pDstIntf(0)
        {
        }

        ~ObjectLinkLocal()
        {
            Base::Disconnect();
        }

        void Connect(IObjectLinksHost *pHost, TIntf *pIntf)
        {
            ESS_ASSERT(pHost);
            ESS_ASSERT(pIntf);

            ESS_ASSERT(m_server.IsEmpty());
            m_server = pHost->getObjectLinksServer();
            m_pDstIntf = pIntf;

            // create proxy for interface
            {
                std::string typeName = ObjectLinkInterfacesFactory::TypeToName<TIntf>(0);
                const IObjectLinkInterfaceMeta *pMeta = ObjectLinkInterfacesFactory::FindMeta(typeName);
                if (pMeta == 0)
                {
                    std::string err = "Empty meta for " + InterfaceName();
                    ESS_HALT(err);
                }

                IObjectLinkInterface *pBasic = pMeta->CreateLocalInterfaceProxy(*this);
                ESS_ASSERT(pBasic != 0);

                TIntf *pProxy = Utils::SafeCastPtr<TIntf>(pBasic);
                ObjectLinkT<TIntf>::setInterface(pProxy);
            }

            // begin connect
            SafeRef<IServerToObjectLink> link(this);
            m_server->ObjectLinkConnectReq(link);
        }

        void Connect(const Domain::FullObjectName &name)
        {
            // find object
            QObject *pObj = Base::Find(name);            
            if (pObj == 0) 
            {
                Base::SendError( new ErrorLocalNamedObjectNotFound(&Base::Owner(), name) );
                return;
            }

            // cast to IObjectLinksHost
            IObjectLinksHost *pHost = dynamic_cast<IObjectLinksHost*>(pObj);
            if (pHost == 0)
            {
                Base::SendError( new ErrorLocalNamedObjectCastToHost(&Base::Owner(), name) );
                return;
            }

            // cast to TIntf
            TIntf *pIntf = dynamic_cast<TIntf*>(pObj);
            if (pIntf == 0) 
            {
                Base::SendError( new ErrorLocalNamedObjectCastToInterface(&Base::Owner(), name) );                
                return;
            }

            // connect
            Connect(pHost, pIntf);
        }


    };

}  // namespace ObjLink

#endif

