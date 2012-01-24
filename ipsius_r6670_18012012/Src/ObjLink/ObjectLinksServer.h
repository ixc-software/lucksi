#ifndef __OBJECTLINKSSERVER__
#define __OBJECTLINKSSERVER__

#include "stdafx.h"
#include "iCore/MsgObject.h"
#include "Domain/IDomain.h"
#include "Domain/DomainClass.h"
#include "ObjLinkInterfaces.h"

namespace ObjLink
{

    class ObjectLinksServer : 
        public iCore::MsgObject,
        public IObjectLinksServer
    {
        typedef std::vector< SafeRef<IServerToObjectLink> > List;

        iCore::MsgObject &m_owner;
        List m_list;

        static iCore::MsgThread& getMsgThread(iCore::MsgObject *pObject)
        {
            ESS_ASSERT(pObject != 0);
            return pObject->getMsgThread();
        }

    // IObjectLinksServer impl
    private:

        void ObjectLinkConnectReq(SafeRef<IServerToObjectLink> &link) 
        {
            // check for dublicates
            List::iterator i = std::find(m_list.begin(), m_list.end(), link);
            ESS_ASSERT( i == m_list.end() );

            // add and send confirmation
            m_list.push_back(link);
            link->ObjectLinkConnectConf();
        }

        void ObjectLinkDisconnect(SafeRef<IServerToObjectLink> &link) 
        {
            // find
            List::iterator i = std::find(m_list.begin(), m_list.end(), link);
            ESS_ASSERT( i != m_list.end() );
            
            m_list.erase(i);
        }

        void ObjectLinkSendMsg(SafeRef<IServerToObjectLink> &link, iCore::MsgBase *pMsg)
        {
            m_owner.PutMsgInQueue(pMsg);
        }

    public:

        ObjectLinksServer(Domain::IDomain &domain, iCore::MsgObject *pOwner) 
            : MsgObject( getMsgThread(pOwner) ), 
            m_owner(*pOwner)
        {
        }

        ~ObjectLinksServer()
        {
            while(!m_list.empty())
            {
                m_list.back()->ObjectLinkDisconnect();
                m_list.pop_back();
            }
        }

        SafeRef<IObjectLinksServer> getInterface()
        {
            return SafeRef<IObjectLinksServer>(this);
        }
    };	
	
}  // namespace ObjLink

#endif



