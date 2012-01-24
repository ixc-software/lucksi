#include "stdafx.h"

#include "DRI/ExecutiveDRI.h"
#include "DRI/DriSessionList.h"

#include "NObjSecRoot.h"
#include "NObjSecProfile.h"

// --------------------------------------------------

namespace DriSec
{

    // potential design problem here, see ~DomainClass()         
    NObjSecProfile::~NObjSecProfile()
    {
        if (getCountRef() <= 0) return;

        // debug
        QString sessInfo = getDomain().ExeDRI().SessionList().GetDetailedInfo();
        QString msg = QString("Security profile %1 in use; Sessions:\n%2").arg(m_name).arg(sessInfo);
        ESS_HALT( msg.toStdString() );
    }

    /*
    QString NObjSecProfile::ExtAllowList( const QString &allowList )
    {
        QString ext = m_context.GetDefaultAllowList(); //  getDomain().Security().GetDefaultAllowList();
        return ext + ";" + allowList;
    } */

}  // namespace DriSec
