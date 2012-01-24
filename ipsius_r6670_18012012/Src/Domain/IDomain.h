#ifndef __IDOMAIN__
#define __IDOMAIN__

#include "Utils/IBasicInterface.h"

namespace Domain
{
	
    class DomainClass;

    class IDomain : public Utils::IBasicInterface
    {
    public:
        virtual DomainClass& getDomain() = 0;
    };
	
	
}  // namespace Domain


#endif







