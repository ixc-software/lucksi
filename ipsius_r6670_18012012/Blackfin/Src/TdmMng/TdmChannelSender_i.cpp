#include "stdafx.h"

#include "TdmChannelSender.h"

namespace TdmMng
{
    bool TdmChannelSender::IsLocalTransit(const std::string &ip) const
    {
        return m_mng.IpIsLocal(ip);
    }
}  // namespace TdmMng
