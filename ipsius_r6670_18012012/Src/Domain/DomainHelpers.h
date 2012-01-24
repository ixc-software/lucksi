#ifndef __DOMAINHELPERS__

#define __DOMAINHELPERS__

#include "stdafx.h"
#include "Utils/IBasicInterface.h"
#include "ObjectName.h"

namespace Domain
{
	
    // результат событий поиска домена
    class IDomainFindResult : public Utils::IBasicInterface
    {
    public:
        // virtual void OnDomainFindResult(DomainName name, DRIUserSide *pDRI) = 0;
    };
	
}  // namespace Domain

#endif

