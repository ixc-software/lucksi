#ifndef __IPSBDEBUG__
#define __IPSBDEBUG__

#include "iPult/CofidecState.h"

namespace PsbCofidec
{
    using iPult::CofidecState;
    using iPult::CsEnum;
            
  	class IPsbDebug : public Utils::IBasicInterface
    {
    public:
    	virtual void WriteLn(const std::string &line) = 0;
    };
           
} // namespace PsbCofidec

#endif