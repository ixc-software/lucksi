#ifndef __TDMCHANNELSENDER__
#define __TDMCHANNELSENDER__

#include "Utils/BidirBuffer.h"
#include "Utils/HostInf.h"
#include "Utils/IVirtualDestroy.h"
#include "Utils/ErrorsSubsystem.h"

#include "ISendDirection.h"
#include "DataPacket.h"
#include "ITdmManager.h"
#include "RtpToTdmBuff.h"
#include "TdmException.h"

namespace TdmMng
{
    using Utils::BidirBuffer;
    using Utils::HostInf;
		       
    // -------------------------------------------------------------

    // TDM channel send subsystem (can be inner for TdmChannel)
    class TdmChannelSender : boost::noncopyable
    {
        ITdmManager &m_mng;
        iRtp::RtpCoreSession &m_rtp;
        Utils::ManagedList<ISendDirection> m_list;
        bool m_pause;

        int Find(const std::string &ip, int port) const;
        int Find(const Utils::SafeRef<ISendSimple> &ref) const;

        void Add(ISendDirection *p);
        void Delete(int index);

        bool IsLocalTransit(const std::string &ip) const;
    public:

        TdmChannelSender(ITdmManager &mng, iRtp::RtpCoreSession &rtp);

        // add send direction, create SendToTdm (if ip is local) or SendToRtp
        void StartSend(const std::string &ip, int port, bool useAlaw);
        void StopSend(const std::string &ip, int port);

        // add send direction to conference
        void StartSend(const Utils::SafeRef<ISendSimple> &ref);
        void StopSend(const Utils::SafeRef<ISendSimple> &ref);

        // suspend all send directions till next StartSend() call
        void PauseSend();

        // send packet to all send dircection
        void Send(DataPacket &dataFromTdm);

        bool Empty() const { return m_list.IsEmpty(); }
        bool Paused() const { return m_pause; }

    };
			
}  // namespace TdmMng

#endif
