#include "stdafx.h"

#include "Domain/DomainClass.h"

#include "ExecutiveDRI.h"
#include "DriSessionList.h"

// ----------------------------------------------------

namespace DRI
{
           
    ExecutiveDRI::ExecutiveDRI( Domain::IDomain &domain ) : 
        iCore::MsgObject( domain.getDomain().getMsgThread() ),
        m_domain(domain), 
        m_currSessionNumber(0),
        m_hintDb(domain),
        m_pActiveTr(0),
        m_metaInfo(*this)
    {
        m_sessionsList.reset( new DriSessionList(domain) );
    }

    // ----------------------------------------------------

    MetaObjectsInfo& ExecutiveDRI::MetaObjInfo()
    {
        return m_metaInfo;
    }

    // ----------------------------------------------------

    ExecutiveDRI::~ExecutiveDRI()
    {
        // nothing 
    }

    // ----------------------------------------------------

    SessionDRI& ExecutiveDRI::GetNewSession( const QString &login, Utils::WeakRef<ISessionDriOwner&> owner, int id )
    {        
        return SessionList().GetNewSession(login, owner, id);
    }

}  // namespace DRI

