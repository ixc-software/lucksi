#include "stdafx.h"

#include "DRI/NamedObjectFactory.h"
#include "DRI/driutils.h"
#include "DRI/ExecutiveDRI.h"
#include "DRI/DriSessionList.h"

#include "NObjSecRoot.h"

// ---------------------------------------------------------

namespace DriSec
{
    using namespace DRI;
       
    QStringList NObjSecRoot::CreateDefaultAllowList()
    {
        std::string typeName = DriUtils::FormatedTypename<NObjSecSpecial>(true);
        INamedObjectType *pT = DRI::NamedObjectFactory::Instance().Find(typeName);
        ESS_ASSERT(pT != 0);

        QStringList sl;
        sl.append( getDomain().ExeDRI().MetaObjInfo().ListMethodsNames( &pT->MetaObject() ) );

        {
            QStringList denyList = NamedObject::GetAllDriActiveMethodList();

            for(int i = 0; i < denyList.size(); ++i)
            {
                QString s = denyList.at(i);
                if ( !sl.removeOne(s) ) ESS_HALT(s.toStdString() + " not in list!");
            }
        }

        return sl; // sl.join(";");
    }

    int NObjSecRoot::LoggedCount( QString login )
    {
        return getDomain().ExeDRI().SessionList().LoggedCount(login);
    }

}  // namespace DriSec

