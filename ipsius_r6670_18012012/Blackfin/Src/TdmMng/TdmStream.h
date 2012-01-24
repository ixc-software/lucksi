#ifndef __TDMSTREAM__
#define __TDMSTREAM__

#include "Utils/ManagedList.h"

#include "DrvAoz/AozShared.h"

#include "ITdmManager.h"
#include "TdmHAL.h"
#include "TdmChannel.h"
#include "TdmChannelsSet.h"
#include "TdmCodecs.h"

namespace TdmMng
{

    // Single TDM stream (with channels)
    class TdmStream : 
        public ITdmOwner,
        boost::noncopyable
    {
        ITdmManager &m_mng;
        const std::string m_name;
        const bool m_useAlaw;
        boost::scoped_ptr<iDSP::ICodec> m_codec;
        Utils::ManagedList<TdmChannel> m_channels;

        TdmStreamStat m_stat;

        boost::scoped_ptr<iLogW::LogSession> m_log;        
        LogRecordTag m_logTag;

        BfTdm::Tdm *m_pTdm;

        BfDev::BfTimerCounter m_timer;

        void DoSimpleLog(const iLogCommon::LogString& msg, const LogRecordTag& tag);
        void SocketPoll(Utils::StatElementForInt &sockStat);

        // in timer ticks - !
        Platform::dword CurrTick()
        {
            return m_timer.GetCounter(); //  Platform::GetSystemTickCount();
        }

    // ITdmOwner impl
    private:

        void TdmFrameRead(TdmReadWrapper &block);
        void TdmFrameWrite(TdmWriteWrapper &block);

    public:

        TdmStream(ITdmManager &mng, 
			Utils::SafeRef<iLogW::ILogSessionCreator> logCreator,
			const std::string &name, const TdmChannelsSet &channels, 
            int sportNum, int sportChannel, bool useAlaw);

        ~TdmStream();

        const std::string& Name() const;

        ITdmChannel* Channel(int number);

        TdmChannel* FindChannelByRtpPort(int rtpPort);

        void BlockProcess(ITdmBlock &block);

        void BlockCollision()
        {
            m_stat.TdmBlockCollision++;            
        }

        const TdmStreamStat& Stat() const
        {
            return m_stat;
        }

        void RouteAozEvent( int chNum, TdmMng::AozLineEvent e );

        void FillRtpPorts(std::vector<int> &ports) const;
        int ActiveChannelsCount() const;

        bool UseAlaw() const { return m_useAlaw; }

        // for statistics get
        BfTdm::Tdm& Tdm()
        {
            ESS_ASSERT(m_pTdm != 0);
            return *m_pTdm;
        }
    };
	
}  // namespace TdmMng

#endif
