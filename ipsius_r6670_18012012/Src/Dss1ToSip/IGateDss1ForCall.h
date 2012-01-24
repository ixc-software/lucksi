#pragma once

#include "stdafx.h"
#include "Utils/IBasicInterface.h"
#include "Utils/SafeRef.h"
#include "iCmpExt/ICmpChannel.h"

namespace Dss1ToSip 
{
    class IDss1Call;
	class GateCallInfo;
    
    class IGateDss1ForCall : public Utils::IBasicInterface
    {
    public:
        virtual void IncomingCall(const IGateDss1ForCall *,
            Utils::SafeRef<IDss1Call> call,
            const ISDN::DssPhoneNumber& calledAddress, 
            const ISDN::DssPhoneNumber& callingAddress) = 0;

		virtual iCmpExt::ICmpChannelCreator& CmpChannelCreator(int interfaceNumber) = 0;

		virtual void SaveCallInfo(int interfaceNumber,
			int channelNumber, Utils::WeakRef<GateCallInfo&> callInfo) = 0;

		virtual QString InterfaceName(int interfaceNumber) const = 0;
		
        virtual void DeleteCall(const IDss1Call *call) =0;
    };
}


