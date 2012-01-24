#include "stdafx.h"

#include "Domain/DomainClass.h"
#include "DriSec/NObjSecRoot.h"

#include "DriSessionList.h"

namespace DRI
{
           
    Utils::SafeRef<DriSec::ISecurity> DriSessionHolder::GetSecurity(Domain::IDomain &domain, 
        const QString &login, /* out */ QString &profileName)
    {
        using namespace DriSec;

        NObjSecUser *pUser = domain.getDomain().Security().FindUser(login);
        ESS_ASSERT(pUser != 0);

        profileName = pUser->Profile().Name().GetShortName();
        return pUser->Profile().GetSecurity();
    }

}  // namespace DRI