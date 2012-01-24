#ifndef RTPCONSTANTS_H
#define RTPCONSTANTS_H

namespace iRtp
{    
    enum RtpConstants
    {
        CRtpVersion = 2,

        CRtpEventHeaderSize = 4,
        CRtcpHeaderSize = 8,
        CRtpHeaderSize  = 12,
        CRtpCsrcSize = 4, // size of identifier the contributing source 

        CRtpSequenceMod = 1 << 16,
        
        CRtpExtensionHeaderSize = 4
    };

    
} // namespace iRtp

#endif
