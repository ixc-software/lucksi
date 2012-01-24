#ifndef _I_DSS_TO_GROUP_H_
#define _I_DSS_TO_GROUP_H_

#include "Utils/IBasicInterface.h"
#include "SetBCannels.h"
#include "BChannelsWrapper.h"

namespace ISDN
{
    class IDssToGroup : Utils::IBasicInterface
    {
    public:
        virtual int CountIntf() const = 0;
        virtual SharedBChansWrapper CaptureAny(int count, int interfaceNumber, bool isOutgoing) = 0;
        virtual SharedBChansWrapper Capture(const SetBCannels& inquredSet, 
			int interfaceNumber, bool exclusive, bool isOutgoing) = 0;                
    };

} // namespace ISDN

#endif
