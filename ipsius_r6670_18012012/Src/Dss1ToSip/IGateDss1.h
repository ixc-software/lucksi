#pragma once

#include "stdafx.h"
#include "Utils/SafeRef.h"
#include "Utils/IBasicInterface.h"
#include "IDss1CallForGate.h"

namespace ISDN
{ 
	class DssWarning;   
	class DssPhoneNumber;   
};

namespace Dss1ToSip 
{
    class IGateDss1 : public Utils::IBasicInterface
    {
    public:
        virtual Utils::SafeRef<IDss1Call> RunOutCall(
			Utils::SafeRef<IDss1CallEvents> user,
            const ISDN::DssPhoneNumber &calledNum, 
            const ISDN::DssPhoneNumber &callingNum) = 0;

        virtual bool isDss1Active() const = 0;
    };

    //-------------------------------------------------------------

    class IGateDss1Events : Utils::IBasicInterface
    {
    public:
        virtual void Activated(const IGateDss1 *) = 0; 
        virtual void Deactivated(const IGateDss1 *) = 0;
        virtual void IncomingCall(const IGateDss1 *,
            Utils::SafeRef<IDss1Call> call,
            const ISDN::DssPhoneNumber &calledAddress,
            const ISDN::DssPhoneNumber &callingAddress) = 0;
    };
}


