#ifndef __ITDMOBSERVER__
#define __ITDMOBSERVER__

#include "Platform/Platform.h"
#include "IUserBlockInterface.h"

namespace BfTdm
{    
    using namespace Platform;   
    
  	class ITdmObserver : Utils::IBasicInterface
    {
    public:
    	virtual void NewBlockProcess(
    	    word sportNum,
    	    IUserBlockInterface &Ch0,
    	    IUserBlockInterface &Ch1,
            bool collision) = 0;    	    
    };
        
}  // namespace Bftdm

#endif
