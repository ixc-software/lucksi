#include "stdafx.h"

#include "TdmChannelSender.h"

namespace TdmMng
{
    bool TdmChannelSender::IsLocalTransit(const std::string &ip) const
    {
        // for emulation
        return false; 
    }
}  // namespace TdmMng

