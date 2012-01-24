#ifndef __TDMTOTDMBUFF__
#define __TDMTOTDMBUFF__

#include "ITdmManager.h"
#include "ITdmSource.h"
#include "DataPacket.h"
#include "tdmtotdmbuffstat.h"

namespace TdmMng
{
    using Utils::BidirBuffer;
    using Utils::HostInf;


    // input for TDM data stream
    class ITdmIn : public Utils::IBasicInterface
    {
    public:
        virtual void OnConnect() = 0;
        virtual void OnData(DataPacket &data) = 0;
        virtual void OnDisconnect() = 0;
    };
	
    // -------------------------------------------------------------
	
    // TDM -> this class (buffer) -> TDM
    class TdmToTdmBuff : 
        public ITdmIn,
        public ITdmSource,
        boost::noncopyable
    {        
        bool m_connected;
        DataPacket m_data;
        TdmToTdmBuffStat m_lastStats;
        TdmToTdmBuffStat m_allStats;
        Platform::byte m_inTimeSlot;

    // ITdmIn impl
    private:

        void OnConnect();

        void OnData(DataPacket &data);

        void OnDisconnect();

    // ITdmSource impl
    private:

        bool WriteToBlock(ITdmWriteWrapper &block);

    public:

        TdmToTdmBuff(ITdmManager &mng, Platform::byte inputTimeSlot);

        bool Active() const { return m_connected; }

        const TdmToTdmBuffStat& getLastStat() const;        

        void getAllTimeStat(TdmToTdmBuffStat& statOut) const;

    };
	
	
}  // namespace TdmMng

#endif
