#ifndef RTPSIGDETECTOR_H
#define RTPSIGDETECTOR_H

#include "iRtp/RtpPcSession.h"
#include "iDSP/g711codec.h"
#include "iDSP/GoertzelDetector.h"
#include "iMedia/iCodec.h"

namespace HiLevelTests
{
    namespace ConfTest
    {
        using boost::scoped_ptr;
        using Platform::dword;

        class IDetectorEvent : Utils::IBasicInterface
        {
        public:
            // notify all freq detect and detect-info if not
            virtual void DetectionFinish(bool ok, const std::string& descr) = 0;
        };

        // ------------------------------------------------------------------------------------

        class RtpSigDetector : iRtp::IRtpPcToUser, public iCore::MsgObject
        {
            enum {CDetectSampleCount = 100, CTimeoutMsec = 100000, CDropPackAtBegin = 100,};

            IDetectorEvent& m_owner;
            bool m_active;
            std::vector<Platform::int16> m_buff;        
            iDSP::GoertzelDetector m_f1Detect;
            iDSP::GoertzelDetector m_f2Detect;
            int m_processed;
            int m_dropCount;
            iCore::MsgTimer m_timeout;

            scoped_ptr<iRtp::RtpPcSession> m_rtp;
            scoped_ptr<iDSP::ICodec> m_codec;
            int m_dBm0;

            bool Detected(iDSP::GoertzelDetector &detector);
            void Timeout(iCore::MsgTimer*);
            void Finish(bool ok, const std::string& descr = "");


            // IRtpPcToUser
        private:
            void RxData(iRtp::SharedBdirBuff buff, iRtp::RtpHeaderForUser header);
            void RxEvent(iRtp::RtpEvent ev, dword timestamp);            
            void RtpErrorInd(iRtp::RtpError er);
            void NewSsrcRegistred(dword newSsrc) {} 

        public:
            RtpSigDetector(IDetectorEvent&, 
                const iMedia::Codec& codec, int f1, int f2, int dBm0,
                iCore::MsgThread& thread,
                iLogW::ILogSessionCreator &log,
                iRtp::RtpInfra& rtpInfra
                );  

            const Utils::HostInf& LocalRtpAddr() const;        

            void StartDetect();

        };

    } // namespace ConfTest

} // namespace HiLevelTests


#endif