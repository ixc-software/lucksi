#ifndef RTPHEADERFORUSER_H
#define RTPHEADERFORUSER_H

#include "Platform/PlatformTypes.h"
#include "RtpPayload.h"

namespace iRtp
{
    using Platform::dword;
    
    struct RtpHeaderForUser
    {
        RtpHeaderForUser(bool marker, dword timestamp, RtpPayload payload)
            : Marker(marker),
            Timestamp(timestamp),
            Payload(payload)
        {}

        virtual ~RtpHeaderForUser(){}

        //void ToStream(std::ostringstream&)...

        bool Marker;
        dword Timestamp;
        RtpPayload Payload;
    };    

} // namespace iRtp

#endif
