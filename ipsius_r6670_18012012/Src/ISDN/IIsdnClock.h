#ifndef CLOCK_H
#define CLOCK_H

#include "Platform/PlatformTypes.h"

#include "Utils/IBasicInterface.h"

namespace ISDN
{
    using Platform::dword;

	class IIsdnClock : public Utils::IBasicInterface
	{
	public:
        
        virtual dword GetTick() const = 0;        
	};	
}//namespace ISDN

#endif
