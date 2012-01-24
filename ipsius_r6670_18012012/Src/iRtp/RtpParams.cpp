#include "stdafx.h"
#include "Utils/MetaConfig.h"
#include "RtpParams.h"

namespace iRtp
{
    void RtpInfraParams::RegisterInMetaConfig(Utils::MetaConfig &cfg, 
        const std::string &shortPreffix)
    {
        QString prefix(shortPreffix.c_str());
        cfg.Add(m_minPort, prefix + "Min port rtp", "min value Rtp port, must be even");
        cfg.Add(m_maxPort, prefix + "Max port rtp", "max value Rtp port, must be odd");
    }
} // namespace iRtp



