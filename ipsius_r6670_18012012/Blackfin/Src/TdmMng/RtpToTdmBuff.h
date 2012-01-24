#ifndef __RTPTOTDMBUFF__
#define __RTPTOTDMBUFF__

#include "Utils/BidirBuffer.h"
#include "Utils/IBasicInterface.h"
#include "Utils/HostInf.h"
#include "Utils/FixedFifo.h"

#include "iRtp/RtpCoreSession.h"

#include "TdmMng/ITdmSource.h"
#include "TdmMng/RtpRecvBufferProfile.h"
#include "TdmMng/RtpToTdmBufferStat.h"

namespace TdmMng
{
    using Utils::BidirBuffer; 
    using Platform::dword;

    // -------------------------------------------------------------------

    // RTP -> this class (buffer) -> TDM
    class RtpToTdmBuff : 
        public ITdmSource,
        boost::noncopyable 
    {
		struct CheckProfile 
		{
			CheckProfile(const RtpRecvBufferProfile& prof) 
			{
				ESS_ASSERT(prof.IsCorrect());
			}
		};
		CheckProfile m_checkProfile;

        enum State 
        {
            st_inactive,   
            st_firstBuffering,
            st_buffering,
            st_sending
        };

        State m_state;       
        RtpToTdmBufferStat m_lastStat;
        RtpToTdmBufferStat m_allStat;
        bool m_statActive;        

        const RtpRecvBufferProfile m_prof;
        dword m_currDepth; // текущая глубина буфферизации (в семплах)
        Utils::FixedFifo<BidirBuffer*> m_buffQueue;
        int m_byteInQueue;        
        const int m_timeSlot;
        

        void IncreaseDepth();
        void DropOldPackets();

    // ITdmSource impl
    private:

        bool WriteToBlock(ITdmWriteWrapper &block);

    public:
               
        RtpToTdmBuff(const RtpRecvBufferProfile& prof, int timeSlot);               

        void PutData(BidirBuffer* pBuff);

        // сумма статистик по всем интервалам активности        
        void  getAllTimeStat(RtpToTdmBufferStat& statOut) const;

        const RtpToTdmBufferStat& getLastStat() const;

        void Start();
        void Stop();     // after Stop() buffer is empty
    };

        
}  // namespace TdmMng

#endif
