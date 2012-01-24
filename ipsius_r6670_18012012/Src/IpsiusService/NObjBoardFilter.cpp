#include "stdafx.h"
#include "NObjBoardFilter.h"

#include "BfBootCore/BroadcastMsg.h"
#include "Domain/DomainClass.h"


namespace 
{
    QString RangeToString(const boost::scoped_ptr<Utils::RangeList>& range)
    {
        if (!range) return "";
        return range->ToString().c_str();
    }
} // namespace 

namespace IpsiusService
{
    using Utils::RangeList;

    NObjBoardFilter::NObjBoardFilter( Domain::NamedObject *pParent, const Domain::ObjectName &name )
        : Domain::NamedObject(&pParent->getDomain(), name, pParent), m_locked(false)
    {
    }       

    // ------------------------------------------------------------------------------------

    QString NObjBoardFilter::getExclude() const
    {
        return RangeToString(m_exclude);
    }

    // ------------------------------------------------------------------------------------

    QString NObjBoardFilter::getInclude() const
    {
        return RangeToString(m_include);
    }

    // ------------------------------------------------------------------------------------

    void NObjBoardFilter::Include(QString rangeList)
    {
        ThrowIfLocked();
        m_include.reset( new RangeList(rangeList.toStdString()) );
    }

    // ------------------------------------------------------------------------------------

    void NObjBoardFilter::Exclude(QString rangeList)
    {
        ThrowIfLocked();
        m_exclude.reset( new RangeList(rangeList.toStdString()) );
    }

    // ------------------------------------------------------------------------------------

    void NObjBoardFilter::Clear()
    {
        ThrowIfLocked();
        m_include.reset();
        m_exclude.reset();
    }

    // ------------------------------------------------------------------------------------

    bool NObjBoardFilter::AcceptedBoard( const BfBootCore::BroadcastMsg& msg )
    {
        bool includeOk = m_include ? m_include->ExistValue(msg.HwNumber) : true;
        bool excludeOk = m_exclude ? !m_exclude->ExistValue(msg.HwNumber) : true;

        return includeOk && excludeOk;
    }

    // ------------------------------------------------------------------------------------

    void NObjBoardFilter::LockChanges( bool lock )
    {
        m_locked = lock;
    }

} // namespace IpsiusService
