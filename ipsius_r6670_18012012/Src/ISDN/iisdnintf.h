#ifndef _I_ISDNINTF_H
#define _I_ISDNINTF_H

#include "Utils/IBasicInterface.h"
#include "SetBCannels.h"
#include "ISDN/IeConstants.h"

namespace ISDN
{
	class IIsdnIntf : public Utils::IBasicInterface
    {
    public:
		virtual int NumberInterface() const = 0;
		virtual SetBCannels CaptureBChannel(int count) = 0;
		virtual bool CaptureBChannel(const SetBCannels &set) = 0;
		virtual void FreeBChannel(const SetBCannels &set) = 0;

        virtual IeConstants::UserInfo getPayload() const = 0;
    };       

} // namespace ISDN

#endif


