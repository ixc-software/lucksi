#ifndef ISENDDIRECTION_H
#define ISENDDIRECTION_H

#include "Utils/IBasicInterface.h"
#include "Utils/SafeRef.h"
#include "Utils/BidirBuffer.h"

namespace TdmMng
{
    class DataPacket;

    class ISendSimple : public Utils::IBasicInterface
    {
    public:
        virtual void Send(const Utils::BidirBuffer &buff) = 0;
    };

    // send to RTP/TDM interface (can be inner for TdmChannelSender)
    class ISendDirection : public Utils::IBasicInterface
    {
    public:
        virtual void Send(DataPacket &pack, bool canDetach) = 0;
        virtual bool Equal(const std::string &ip, int port) const = 0;
        virtual bool Equal(const Utils::SafeRef<ISendSimple> &ref) const = 0;
    };

} // namespace TdmMng

#endif
