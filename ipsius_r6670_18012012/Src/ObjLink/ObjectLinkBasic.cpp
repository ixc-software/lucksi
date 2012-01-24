#include "stdafx.h"
#include "ObjectLinkBasic.h"
#include "Domain/DomainClass.h"

// --------------------------------------

namespace ObjLink
{

    ObjectLinkBasic::ObjectLinkBasic(IObjectLinkOwnerNotify &owner) 
        : iCore::MsgObject(owner.getDomain().getMsgThread()),
        m_owner(owner), m_connected(false)
    {
        ESS_ASSERT(getMsgThread().InCurrentThreadContext());
    }

    bool ObjectLinkBasic::NameIsLocal(const Domain::FullObjectName &name, IObjectLinkOwner *pOwner)
    {
        ESS_ASSERT(pOwner);
        return ( name.Domain() == pOwner->getDomain().getName() );
    }

    QObject* ObjectLinkBasic::Find(const Domain::ObjectName &name)
    {
        return m_owner.getDomain().FindFromRoot(name);
    }

	
}  // namespace ObjLink


