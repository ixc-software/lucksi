#ifndef __ITDMMANAGER__
#define __ITDMMANAGER__

#include "Utils/IBasicInterface.h"
#include "iRtp/RtpParams.h"
#include "iRtp/IRtpInfra.h"
#include "Platform/PlatformTypes.h"

#include "TdmEvents.h"

namespace TdmMng
{
    using boost::shared_ptr;
    using Platform::word;

    class TdmHAL;
    class TdmChannel;
    class DataPacketBody;
    class IrqInfoMsg;
    class RtpRecvBufferProfile;
    class EventsQueue;

    /* Shared TdmManager functionality: 
            - logging, 
            - socks creation (ports use) 
            - find channel by port
            - HAL access
            - handle HAL IRQ
            - memory pool / packet body creation
     */
    class ITdmManager : Utils::IBasicInterface
    {
    public:

        // use in TdmStream
        virtual TdmHAL& HAL() = 0;

        // use in TdmChannelSender
        virtual TdmChannel* FindChannelByRtpPort(int rtpPort) = 0;
        virtual bool IpIsLocal(const std::string &ip) const = 0;

        // async events register
        virtual EventsQueue& Queue() = 0;

        // use in TdmChannel
        virtual iRtp::IRtpInfra& getRtpInfra() = 0;
        virtual const iRtp::RtpParams& getGeneralRtpParams() const = 0;
        virtual const RtpRecvBufferProfile& getBufferingProf() const = 0;        
    };


}  // namespace TdmMng

#endif
