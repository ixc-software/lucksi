#include "stdafx.h"
#include "BoolALawToRtpPayload.h"

namespace TdmMng
{
    iRtp::RtpPayload BoolALawToRtpPayload(bool useALaw)
    {
        return (useALaw) ? (iRtp::PCMA) : (iRtp::PCMU);    
    }
} // namespace TdmMng

