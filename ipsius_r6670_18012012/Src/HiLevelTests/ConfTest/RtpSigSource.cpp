#include "stdafx.h"
#include "RtpSigSource.h"

// For Debug.
namespace
{
    const bool CUseDeterminateData = false;

    using boost::scoped_ptr;    

    // Source of determinate data.    
    class StaticData: boost::noncopyable,
        public iRtpUtils::IDataSrcForRtpTest     
    {        
        QByteArray m_cash;
        int m_v1;
        int m_v2;

        // IDataSrcForRtpTest
    private:
        void Setup(const iMedia::Codec &codecType)
        {                 
            scoped_ptr<iDSP::ICodec> codec;
            if (codecType == iMedia::Codec::ALaw_8000)
                codec.reset( new iDSP::CodecAlow() );
            else                
                codec.reset( new iDSP::CodecUlow() );

            Platform::byte v1, v2;            

            v1 = codec->Encode(m_v1);
            v2 = codec->Encode(m_v2);

            for (int i = 0; i < 160; ++i)
            {                
                m_cash.push_back( i%2 > 0 ? v1 : v2 );                
            }            
        }

        QByteArray Read(int size)
        {
            return m_cash;
        }

    public:
        StaticData(int v1, int v2) : m_v1(v1), m_v2(v2)
        {                           
        }
    };

} // namespace

// ------------------------------------------------------------------------------------

namespace HiLevelTests
{
    namespace ConfTest
    {

        RtpSigSource::RtpSigSource(
            const Utils::HostInf &dst,
            const iMedia::Codec &codec, int freq, int dBm0, 
            iCore::MsgThread &thread, 
			iLogW::ILogSessionCreator &log,
            iRtp::RtpInfra &rtpInfra
            )
        {
            iRtpUtils::GenerateSinusFactory genFactory(dBm0, freq, true);

            iRtp::RtpParams prof;			

            m_rtp.reset( new iRtp::RtpPcSession(thread, log, prof, rtpInfra, *this, dst) );

            m_rtp->setPayload( iRtp::RtpPayload(codec.getPayloadType()) );            

            if (!CUseDeterminateData)
            {
                m_srcSender.reset( genFactory.CreateTest(thread, m_rtp.get()) );            
            }
            else // debug
            {                
                int v1 = 50;
                int v2 = 100;

                m_srcSender.reset(
                    new iRtpUtils::PlayData(thread, m_rtp.get(), new StaticData(v1, v2))
                    );                
            }                        

            m_srcSender->StartSend(codec);
        }

    } // namespace ConfTest

} // namespace HiLevelTests