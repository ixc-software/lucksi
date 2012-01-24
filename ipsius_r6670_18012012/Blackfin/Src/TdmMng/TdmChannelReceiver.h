#ifndef __TDMCHANNELRECEIVER__
#define __TDMCHANNELRECEIVER__

#include "TdmChannelStatistic.h"
#include "BoolALawToRtpPayload.h"

#include "RtpToTdmBuff.h"
#include "TdmToTdmBuff.h"

namespace TdmMng
{
    
    class TdmChannelReceiver
    {
        const iRtp::RtpPayload m_rtpCodec;

        bool m_enabled;

        TdmToTdmBuff m_inFromTdm;                   // recv from another TDM, (highest priority) 
        Utils::SafeRef<IRtpLikeTdmSource> m_conf;   // recv from Conference 
        RtpToTdmBuff m_inFromRtp;                   // recv from RTP session  (lowest priority)

        void PutToBuffer(BidirBuffer *pBuff)
        {
            if (m_enabled)
            {
                m_inFromRtp.PutData(pBuff);
            }
            else
            {
                // stat update
                // ... 

                delete pBuff;
            }
        }
        
    public:
        
        TdmChannelReceiver(ITdmManager &mng, int chNumber, bool useAlaw) :
          m_rtpCodec( BoolALawToRtpPayload(useAlaw) ),  
          m_enabled(false),
          m_inFromTdm(mng, chNumber),
          m_inFromRtp(mng.getBufferingProf(), chNumber)
        {
        }

        bool Enabled() const { return m_enabled; }

        void Start(Utils::SafeRef<IRtpLikeTdmSource> conf 
            = Utils::SafeRef<IRtpLikeTdmSource>())
        {
            if ( m_enabled )
            {
                ESS_THROW_T(TdmException, terAlreadyRecvEnabled);
            }

            m_enabled = true;
            m_conf = conf;
            m_inFromRtp.Start(); 
        }

        void Stop(Utils::SafeRef<IRtpLikeTdmSource> conf 
            = Utils::SafeRef<IRtpLikeTdmSource>())
        {
            if (!m_enabled)
            {
                ESS_THROW_T(TdmException, terAlreadyRecvDisabled);
            }

            if (conf.IsEmpty())
            {
                ESS_ASSERT(m_conf.IsEmpty());
            }
            else
            {
                ESS_ASSERT(m_conf == conf);
                m_conf.Clear();                
            }

            m_enabled = false;
            m_inFromRtp.Stop();
        }

        // this call sync'ed with TDM write cycle
        ITdmSource& GetTdmSource()
        {
            ESS_ASSERT(m_enabled);

            if (m_inFromTdm.Active()) return m_inFromTdm;

            if (!m_conf.IsEmpty())    
            {
                BidirBuffer *p = m_conf->DetachData();
                if (p != 0) PutToBuffer(p);
            }

            return m_inFromRtp;
        }

        ITdmIn& GetTdmInput()
        {
            return m_inFromTdm;
        }

        void PutRtpData(BidirBuffer *pBuff, iRtp::RtpHeaderForUser header)
        {
            bool badPayload = ( header.Payload != m_rtpCodec );
            bool confActive = ( !m_conf.IsEmpty() );

            if (badPayload || confActive)
            {
                // add stats
                // ...

                // drop data
                delete pBuff;
                return;
            }

            PutToBuffer(pBuff);
        }

        void GetAllTimeStat(TdmChannelStatistic &stat) const
        {
            m_inFromRtp.getAllTimeStat( stat.RtpBuffStat );
            m_inFromTdm.getAllTimeStat( stat.TdmBuffStat );
        }

        void GetLastCallStat(TdmChannelStatistic &stat) const 
        {
            stat.RtpBuffStat = m_inFromRtp.getLastStat();
            stat.TdmBuffStat = m_inFromTdm.getLastStat();
        }

        
    };
    
    
}  // namespace TdmMng

#endif
