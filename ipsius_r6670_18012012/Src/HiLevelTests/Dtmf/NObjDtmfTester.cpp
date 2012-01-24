#include "stdafx.h"
#include "NObjDtmfTester.h"
#include "iDSP/DtmfGenerator.h"
#include "iRtpUtils/IDataSrcForRtpTest.h"
#include "iCmpExt/ICmpChannel.h"
#include "iRtp/RtpPcSession.h"
#include "iRtpUtils/PlayData.h"
#include "Domain/DomainClass.h"
#include "Utils/WeakRef.h"

namespace 
{
    const std::string CTestDtmfSeq = "AD1790*#";
    const int CResponseTimeout = 40 * 1000; //' msec
    const int CSilentBeforeSendMs = 0; // 4 * 1000;

     class DtmfSource : public iRtpUtils::IDataSrcForRtpTest
     {       
         //const iMedia::Codec m_codec;
         int m_silentSamples;
         boost::scoped_ptr<iDSP::ICodec> m_codec;
         iDSP::DtmfGenerator m_gen;

     // TestRtpWithSip::IDataSrcForRtpTest
     private:            

         void Setup(const iMedia::Codec &codec)
         {
             if (codec == iMedia::Codec::ALaw_8000)
                 m_codec.reset(new iDSP::CodecAlow);
             if (codec == iMedia::Codec::ULaw_8000)
                 m_codec.reset(new iDSP::CodecUlow);
             ESS_ASSERT(m_codec);
         }

         QByteArray Read(int size)
         {   
             ESS_ASSERT(m_codec != 0);

             QByteArray buff;      
             char null =  m_codec->Encode(0);
 
             while(buff.length() < size)
             {
                 if (m_silentSamples > 0)
                 {
                    buff.push_back(null);
                    m_silentSamples--;
                    continue;
                 }

                 if (m_gen.RestSampleCount())
                 {
                     buff.push_back( m_codec->Encode( m_gen.NextSample() ) );
                     continue;
                 }

                 buff.push_back(null); 
             }         
 
             return buff;
         }
 
     public:         

         DtmfSource() : m_silentSamples(CSilentBeforeSendMs * 8)
         {
             m_gen.Add(CTestDtmfSeq);             
         }
     };
    
} // namespace 

// ------------------------------------------------------------------------------------

namespace HiLevelTests
{
    using Platform::dword;

    namespace  Dtmf
    {        

        class NObjDtmfTester::DtmfTesterImpl 
            : iCmpExt::ICmpChannelEvent, public iRtp::IRtpPcToUser
        {
            NObjDtmfTester& m_owner;  
            Utils::WeakRefHost m_weakHost;

            boost::scoped_ptr<iCmpExt::ICmpChannel> m_genSideCtrl; // rtp generator side
            boost::scoped_ptr<iCmpExt::ICmpChannel> m_detectorSideCtrl; // dtmf detector side
            
            boost::scoped_ptr<iRtp::RtpPcInfra> m_rtpInfra;
            boost::scoped_ptr<iRtp::RtpPcSession> m_rtp;             
            boost::scoped_ptr<iRtpUtils::IRtpTest> m_srcSender;            

            std::string m_dtmfRecBuff;

            // ------------------------------------------------------------------------------------

            iCmpExt::CmpChannelId GenSideCtrlId() const {return &m_genSideCtrl;}
            iCmpExt::CmpChannelId DetectSideCtrlId() const {return &m_detectorSideCtrl;}

            // ------------------------------------------------------------------------------------

            void CreateRtpSource(const Utils::HostInf &dst, const iMedia::Codec &codec) // create sender
            {   
                iRtp::RtpParams prof;
                // prof.CoreTraceInd = true;
                // prof.trace = m_owner.
                
				m_rtp.reset(new iRtp::RtpPcSession(m_owner.getMsgThread(), 
					m_owner.Log(), 
					prof, 
					m_rtpInfra->Infra(),
					*this,
					dst));

                m_rtp->setPayload(
                    iRtp::RtpPayload(codec.getPayloadType())
                    );
                
                m_srcSender.reset( new iRtpUtils::PlayData(m_owner.getMsgThread(), m_rtp.get(), new DtmfSource) );
            }
            
            // ------------------------------------------------------------------------------------            

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
                m_owner.TestFinished(false, er.Desc().c_str());
            }
            void NewSsrcRegistred(dword newSsrc)
            {                
            }      

            // ------------------------------------------------------------------------------------

        // iCmpExt::ICmpChannelEvent impl:
        private:
            void ChannelCreated(iCmpExt::ICmpChannel *cmpChannel, iCmpExt::CmpChannelId id)
            {        
                ESS_ASSERT(id == DetectSideCtrlId() || id == GenSideCtrlId());

                if (GenSideCtrlId() == id)
                {
                    ESS_ASSERT(m_genSideCtrl == 0);
                    m_genSideCtrl.reset(cmpChannel);
                }
                
                if (DetectSideCtrlId() == id)
                {
                    ESS_ASSERT(m_detectorSideCtrl == 0);
                    m_detectorSideCtrl.reset(cmpChannel);
                }                

                if (!m_detectorSideCtrl || !m_detectorSideCtrl) return;

                // start
                iMedia::Codec codec = m_genSideCtrl->UseAlaw() ? iMedia::Codec::ALaw_8000 : iMedia::Codec::ULaw_8000;

                CreateRtpSource(m_genSideCtrl->ChannelRtpAddr(), codec); 

                m_genSideCtrl->StartRecv();
                m_detectorSideCtrl->DtmfDetect(true, iCmp::FrOn);                                               

                m_srcSender->StartSend(codec);                
            }            

            
            void ChannelDeleted(iCmpExt::CmpChannelId id, const std::string &desc)
            {
                ESS_ASSERT(id == DetectSideCtrlId() || id == GenSideCtrlId());                

                m_genSideCtrl.reset();
                m_detectorSideCtrl.reset();
                m_owner.TestFinished(false, desc.c_str());
            }            

            void DtmfEvent(const iCmpExt::ICmpChannel *p, const std::string& dtmfData)             
            {                              
                TUT_ASSERT(m_detectorSideCtrl.get() == p);
                m_dtmfRecBuff.append(dtmfData);                

                if ( m_dtmfRecBuff != CTestDtmfSeq.substr(0, m_dtmfRecBuff.size()) )
                {
                    std::string msg;
                    msg = "Recv " + m_dtmfRecBuff + " vs " + CTestDtmfSeq;
                    m_owner.TestFinished(false, msg.c_str());
                }

                if (m_dtmfRecBuff.size() == CTestDtmfSeq.size()) // == if (m_dtmfRecBuff == CTestDtmfSeq) 
                {
                    m_owner.TestFinished(true, "Dtmf test complete");
                }
            } 

            void AsyncCreateChannel(iCmpExt::CmpChannelId id, iCmpExt::ICmpChannelCreator& creator)
            {                                     
                creator.CreateCmpChannel(
                    id, m_owner.getChannel(),
                    m_weakHost.Create<iCmpExt::ICmpChannelEvent&>(*this),
                    m_owner.Log().LogCreatorWeakRef()
                    );
            }

            // ------------------------------------------------------------------------------------
        
        public:
            DtmfTesterImpl(iCmpExt::ICmpChannelCreator& b1, iCmpExt::ICmpChannelCreator& b2, NObjDtmfTester& owner):
                m_owner(owner)
            {
                m_rtpInfra.reset( new iRtp::RtpPcInfra(owner.getMsgThread(), owner.RtpInfraParams()) );

                AsyncCreateChannel(GenSideCtrlId(), b1);
                AsyncCreateChannel(DetectSideCtrlId(), b2);
            }            
        };

        // ------------------------------------------------------------------------------------
         
        void NObjDtmfTester::Run(DRI::IAsyncCmd* pAsyncCmd, QString b1, QString b2) /* cast, create rtpInfra, createImpl */
        {               
            m_impl.reset(new DtmfTesterImpl(CastIntf(b1), CastIntf(b2), *this));
            
            AsyncBegin(pAsyncCmd);            
            m_timer.Start(CResponseTimeout);
        }

        // ------------------------------------------------------------------------------------

        iCmpExt::ICmpChannelCreator& NObjDtmfTester::CastIntf( QString driName )
        {
            QString err; 
            iCmpExt::ICmpChannelCreator* p = getDomain().FindFromRoot<iCmpExt::ICmpChannelCreator>(driName, &err);
            if (p == 0) ThrowRuntimeException(err);
            return *p;
        }      

        // ------------------------------------------------------------------------------------

        // move ~scoped_ptr<> under impl definition
        NObjDtmfTester::~NObjDtmfTester()
        {
        }     

        // ------------------------------------------------------------------------------------

        void NObjDtmfTester::TestFinished( bool ok, QString descr )
        {                        
            PutMsg(this, &NObjDtmfTester::OnFinished, FinishInfo(descr, ok));
        }

        // ------------------------------------------------------------------------------------

        void NObjDtmfTester::OnFinished( const FinishInfo& info )
        {
            if (!AsyncActive()) return;

            m_impl.reset();            
            m_timer.Stop();
            AsyncComplete(info.Ok, info.Descr);
        }

        // ------------------------------------------------------------------------------------

        iRtp::RtpInfraParams NObjDtmfTester::RtpInfraParams() const
        {
            return iRtp::RtpInfraParams("127.0.0.1", m_minRtpPort, m_maxRtpPort);
        }

        // ------------------------------------------------------------------------------------

        NObjDtmfTester::NObjDtmfTester( Domain::IDomain *pDomain, const Domain::ObjectName &name ):
            Domain::NamedObject(pDomain, name), 
            m_channel(-1), 
            m_timer(this, &NObjDtmfTester::OnTimeout)
        {
            m_minRtpPort = 1000;
            m_maxRtpPort = 10000;           
        }

        // ------------------------------------------------------------------------------------

        void NObjDtmfTester::OnTimeout( iCore::MsgTimer*pTimer )
        {
            ESS_ASSERT(pTimer == &m_timer);
            if (!m_impl) return;
            TestFinished(false, "Failed by timeout");
        }
        
    } // namespace  Dtmf

} // namespace HiLevelTests

