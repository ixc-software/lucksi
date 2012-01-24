#ifndef RTPPCINFRA_H
#define RTPPCINFRA_H

#include "stdafx.h"
#include "Utils/BidirBufferCreator.h"
#include "RtpInfra.h"

namespace iRtp
{   
    class RtpPcInfra : boost::noncopyable
    {                        
        Utils::BidirBufferCreator<> m_buffCreator;
        RtpInfra m_infra;            
    public:       
        RtpPcInfra(iCore::MsgThread &thread, 
            const RtpInfraParams &params);
        RtpInfra &Infra() {  return m_infra; };
    };
} // namespace iRtp

#endif
