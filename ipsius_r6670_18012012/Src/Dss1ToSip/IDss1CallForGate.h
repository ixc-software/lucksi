#pragma once

#include "Utils/IBasicInterface.h"
#include "iMedia/iCodec.h"
#include "iCmpExt/ICmpChannel.h"

namespace iRtpExt	{ class CallToRtp; }
namespace ISDN	{	class DssCause; }
namespace iReg	{	class CallSideRecord;	}

namespace Dss1ToSip
{
	class IDss1CallEvents;

    class IDss1Call : public Utils::IBasicInterface
    {
    public:
        virtual void SetUserCall(Utils::SafeRef<IDss1CallEvents> user) = 0;
        virtual std::string CallName() const = 0;
        virtual void Alert(const IDss1CallEvents *id) = 0;
        virtual void Connect(const IDss1CallEvents *id) = 0;
        virtual void Release(const IDss1CallEvents *id, int rejectCode = 0) = 0;
		virtual iCmpExt::ICmpChannel &Rtp(const IDss1CallEvents *id) = 0;
    };
	
	// --------------------------------------------------------------------------

	class IDss1CallEvents : public Utils::IBasicInterface
	{
	public:
		virtual void Created(const IDss1Call *) = 0; 
		virtual void Alerted(const IDss1Call *) = 0; 
		virtual void Connected(const IDss1Call *) = 0; 
		virtual void Disconnented(const IDss1Call *,
			boost::shared_ptr<const ISDN::DssCause> pCause) = 0;

		virtual iReg::CallSideRecord& CallRecord() = 0;
	};

}


