#include "stdafx.h"
#include "RtpSigDetector.h"

namespace HiLevelTests
{
    namespace ConfTest
    {
        RtpSigDetector::RtpSigDetector(IDetectorEvent& owner, 
            const iMedia::Codec& codec, int f1, int f2, int dBm0,
            iCore::MsgThread& thread,
            iLogW::ILogSessionCreator &log,
            iRtp::RtpInfra& rtpInfra
            )
            : iCore::MsgObject(thread),
            m_owner(owner),
            m_active(false),
            m_f1Detect(f1),
            m_f2Detect(f2), 
            m_dBm0(dBm0),
            m_processed(0),
            m_dropCount(0),
            m_timeout(this, &RtpSigDetector::Timeout)
        {       
            if (codec == iMedia::Codec::ALaw_8000)
                m_codec.reset( new iDSP::CodecAlow() );
            else
                if (codec == iMedia::Codec::ULaw_8000)
                    m_codec.reset( new iDSP::CodecUlow() );
                else
                    ESS_UNIMPLEMENTED;


            m_rtp.reset(new iRtp::RtpPcSession(thread, log, iRtp::RtpParams(), rtpInfra, *this));
            m_rtp->setPayload(
                iRtp::RtpPayload(codec.getPayloadType())
                );  
            //StartDetect();
        } 

        // ------------------------------------------------------------------------------------

        bool RtpSigDetector::Detected( iDSP::GoertzelDetector& detector )
        {
            int dBm0 = detector.Release_dBm0();                
            return abs(dBm0 - m_dBm0) <= iDSP::GoertzelDetector::CdBm0_Precision;        
        }

        // ------------------------------------------------------------------------------------

        void RtpSigDetector::Finish( bool ok, const std::string& descr /*= ""*/ )
        {
            m_active = false;
            m_owner.DetectionFinish(ok, descr);
        }

        // ------------------------------------------------------------------------------------

        void RtpSigDetector::RxData( iRtp::SharedBdirBuff buff, iRtp::RtpHeaderForUser header )
        {
            if (!m_active) return;

            if (m_dropCount < CDropPackAtBegin) 
            {
                ++m_dropCount;
                return;
            }

            m_buff.clear();

            for (int i = 0; i < buff->Size(); ++i)
            {
                Platform::int16 val = m_codec->Decode(buff->At(i));
                m_buff.push_back( val );
                if (m_buff.size() == CDetectSampleCount) break;
            }                                

            m_f1Detect.Process(m_buff);
            m_f2Detect.Process(m_buff);

            m_processed += m_buff.size();
            if (m_processed < CDetectSampleCount) return;

            m_processed = 0;
            bool f1 = Detected(m_f1Detect);
            bool f2 = Detected(m_f2Detect);        

            if (f1 && f2) Finish(true, "Detection complete");   
            else
            {
                std::string err;
                if (!f1) err.append("F1 not detected");
                if (!f2) err.append(", F2 not detected");            
                Finish(false, err);
            }
        }

        // ------------------------------------------------------------------------------------

        void RtpSigDetector::RxEvent( iRtp::RtpEvent ev, dword timestamp )
        {
            if (!m_active) return;
            ESS_UNIMPLEMENTED;
        }

        // ------------------------------------------------------------------------------------

        void RtpSigDetector::RtpErrorInd( iRtp::RtpError er )
        {
            if (!m_active) return;
            Finish(false, er.Desc());        
        }

        // ------------------------------------------------------------------------------------

        void RtpSigDetector::Timeout( iCore::MsgTimer* )
        {
            if (!m_active) return;
            m_f1Detect.Release();
            m_f2Detect.Release();
            Finish(false, "Detect timeout");
        }

        // ------------------------------------------------------------------------------------

        const Utils::HostInf& RtpSigDetector::LocalRtpAddr() const
        {        
            return m_rtp->getLocalRtpAddr();
        }

        // ------------------------------------------------------------------------------------

        void RtpSigDetector::StartDetect()
        {
            ESS_ASSERT(!m_active);
            m_active = true;
            m_dropCount = 0;
            m_timeout.Start(CTimeoutMsec);
        }

    } // namespace ConfTest

} // namespace HiLevelTests

