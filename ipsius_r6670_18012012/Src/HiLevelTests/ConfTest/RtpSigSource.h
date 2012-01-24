#ifndef RTPSIGSOURCE_H
#define RTPSIGSOURCE_H

#include "iRtp/RtpPcSession.h"
#include "iRtpUtils/GenerateSinus.h"

namespace HiLevelTests
{
    namespace ConfTest
    {
        using boost::scoped_ptr;
        using Platform::dword;                          

        // Rtp + generator
        class RtpSigSource : iRtp::IRtpPcToUser
        {		
            scoped_ptr<iRtp::RtpPcSession> m_rtp;   
            scoped_ptr<iRtpUtils::IRtpTest> m_srcSender;

        // IRtpPcToUser
        private:
            void RxData(iRtp::SharedBdirBuff buff, iRtp::RtpHeaderForUser header)
            {
                ESS_UNIMPLEMENTED;
            }

            void RxEvent(iRtp::RtpEvent ev, dword timestamp)
            {
                ESS_UNIMPLEMENTED;
            }            

            void RtpErrorInd(iRtp::RtpError er)
            {
                ESS_UNIMPLEMENTED;
                // todo
                //m_owner.TestFinished(false, er.Desc().c_str());
            }

            void NewSsrcRegistred(dword newSsrc)
            {                
            } 

        public:
            RtpSigSource(
                const Utils::HostInf &dst, const iMedia::Codec &codec, int freq, int dBm0,
                iCore::MsgThread &thread,
                iLogW::ILogSessionCreator &log,
                iRtp::RtpInfra &rtpInfra
                );
        };

    } // namespace ConfTest

} // namespace HiLevelTests


#endif