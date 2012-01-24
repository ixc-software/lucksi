#ifndef __TDMMANAGER__
#define __TDMMANAGER__

#include "Utils/ManagedList.h"
#include "Utils/BidirBufferCreator.h"
#include "iCore/MsgThread.h"
#include "iCore/MsgObject.h"
#include "iRtp/RtpInfra.h"
#include "BfDev/BfTimerCounter.h"
#include "SndMix/IConfToTdm.h"

#include "TdmManagerProfile.h"
#include "TdmHAL.h"
#include "TdmStream.h"
#include "ITdmManager.h"
#include "TdmChannelsSet.h"


namespace SBProto
{
    class ISafeBiProtoForSendPack;
}

namespace SndMix
{
    class ConferenceMng;
}

namespace TdmMng
{
    using iCore::MsgObject;    

    class ITdmChannel;
    
    // main class for TDM subsystem
    class TdmManager : boost::noncopyable,
        public MsgObject,
        public ITdmManager,
        public ITdmHalEvents,
        public SndMix::IConfToTdm
    {        
    
        struct ProfileVerify
        {
            ProfileVerify(const TdmManagerProfile &prof)
            {
                ESS_ASSERT(prof.IsCorrect());
            };
        };
        ProfileVerify m_profileVerify;  // dummi
        const TdmManagerProfile m_prof; // todo хранить по значению        

		// log fields:
		boost::scoped_ptr<iLogW::LogSession> m_log; 
		iLogW::LogRecordTag m_tagUserCmd;
		iLogW::LogRecordTag m_tagError;

        const iRtp::RtpParams m_generalRtpParams; // параметры для всех Rtp-сессий        
        
        Utils::BidirBufferCreator<> m_buffCreator;
        
        iRtp::RtpInfra m_rtpInfra;

        std::string m_localIp;
        
        Utils::ManagedList<TdmStream> m_streams;
        boost::scoped_ptr<SndMix::ConferenceMng> m_conference;

        EventsQueue m_eventQueue;

        void StreamsSocketPoll();

        void BodyIRQ(IrqInfoMsg &info);
        void ProcessIRQ(shared_ptr<IrqInfoMsg> info);

    // SndMix::IConfToTdm impl
    private:

        void GetDeviceInfo(const std::string &streamName, 
            /* out */ int &tdmBlockSize, bool &aLow);

        void BindToConference(const std::string &streamName, int chNum,
            const Utils::SafeRef<TdmMng::ISendSimple> &send,
            const Utils::SafeRef<TdmMng::IRtpLikeTdmSource> &recv);

        void UnbindFromConference(const std::string &streamName, int chNum,
            const Utils::SafeRef<TdmMng::ISendSimple> &send,
            const Utils::SafeRef<TdmMng::IRtpLikeTdmSource> &recv);


    // ITdmHalEvents impl
    private:

        void OnTdmHalIrq(int sportNum, ITdmBlock &block);
        void OnTdmHalBlockCollision();

    // ITdmManager impl
    private:

        TdmHAL& HAL() { return m_prof.Hal; }

        TdmChannel* FindChannelByRtpPort(int rtpPort);

        bool IpIsLocal(const std::string &ip) const;

        BfDev::VdkThreadDcb& getDcbQueue();

        iRtp::IRtpInfra& getRtpInfra();

        const iRtp::RtpParams& getGeneralRtpParams() const;

        const RtpRecvBufferProfile& getBufferingProf() const;

        EventsQueue& Queue() { return m_eventQueue; }

    public:

        TdmManager(const TdmManagerProfile &profile);

        ~TdmManager();

        TdmStream* RegisterStream(const std::string &name, const TdmChannelsSet &channels, 
            int sportNum, int sportChannel, bool useAlaw);

        TdmStream* Stream(const std::string &name, bool throwOnNull = false); 
        ITdmChannel& Channel(const std::string &streamName, int chNumber);

        SndMix::ConferenceMng& ConferenceMng()
        {
            ESS_ASSERT(m_conference != 0);
            return *m_conference;
        }

        void SendAllEventsFromQueue(SBProto::ISafeBiProtoForSendPack &proto,
                                    TdmMng::IIEventsQueueItemDispatch &hook);

    };
	
}  // namespace TdmMng


#endif
