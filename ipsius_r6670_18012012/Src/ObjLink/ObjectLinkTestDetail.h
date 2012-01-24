#ifndef __OBJECTLINKTESTDETAIL__
#define __OBJECTLINKTESTDETAIL__

#include "stdafx.h"
#include "iCore/MsgObject.h"
#include "Domain/DomainClass.h"
#include "ObjectLink.h"
#include "ObjectLinksServer.h"

namespace ObjLink
{
    using boost::shared_ptr;

    class ForceMoc_ObjectLinkTestDetail : QObject { Q_OBJECT }; 
	
    class IDialInterface : public IObjectLinkInterface
    {
    public:
        virtual void Dial(shared_ptr<QString> number) = 0;
        virtual void SetRegionCode(int code) = 0;
        virtual void EndCall() = 0;
    };
	
}  // namespace ObjLink

#endif
