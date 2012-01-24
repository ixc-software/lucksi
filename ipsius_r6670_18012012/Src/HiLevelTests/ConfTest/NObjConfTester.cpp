#include "stdafx.h"
#include "NObjConfTester.h"

#include "Utils/IBasicInterface.h"
#include "Utils/SafeRef.h"

#include "iCore/MsgThreadFn.h"
#include "iCmpExt/ICmpChannel.h"
#include "iCmpExt/iCmpConfPoint.h"
#include "SndMix/MixerCodec.h"
#include "Domain/DomainClass.h"
#include "SndMix/MixerCodec.h"

#include "RtpSigSource.h"
#include "RtpSigSource.h"
#include "RtpSigDetector.h"

namespace 
{    

    const int CTestTimeout = 5 * 1000;

    // ------------------------------------------------------------------------------------

    std::string PayloadToCodecName(iRtp::RtpPayload payload)
    {                        
        return SndMix::CodecMng::Find(payload, true)->Name(); 
    }

    // ------------------------------------------------------------------------------------

    iMedia::Codec PayloadToCodec(iRtp::RtpPayload payload)
    {
        ESS_ASSERT(payload == iRtp::PCMA || payload == iRtp::PCMU);
        return payload == iRtp::PCMA ? iMedia::Codec::ALaw_8000 : iMedia::Codec::ULaw_8000;        
    }    

} // namespace

// ------------------------------------------------------------------------------------

namespace HiLevelTests
{
    namespace ConfTest
    {
        using boost::scoped_ptr;
        using namespace iCmpExt;

        class NObjConfTester::TestImpl :         
            ICmpConfRtpPointEvents, // create rtp point
            ICmpChannelEvent, // create cmp channel,        
            IDetectorEvent
        {        
            enum 
            {
                CdBm0 = -20, // уровень однотональных источников            
            }; 

            NObjConfTester& m_owner;        
            Utils::WeakRefHost m_selfWeakHost;
            scoped_ptr<iRtp::RtpPcInfra> m_rtpInfra;        

            // cmp channel - left source        
            scoped_ptr<iCmpExt::ICmpChannel> m_lCmpIntf; 
            scoped_ptr<RtpSigSource> m_rtpChannSrc; 

            scoped_ptr<iCmpExt::ICmpConf> m_conf;

            scoped_ptr<iCmpExt::ICmpConfPoint> m_tdmPoint;

            scoped_ptr<iCmpExt::ICmpConfPoint> m_genPoint;

            // conf rtp point as source + rtp data source
            scoped_ptr<iCmpExt::ICmpConfRtpPoint> m_rtpPoint;
            scoped_ptr<RtpSigSource> m_rtpPointSrc; 

            // conf rtp point as receiver + rtp data detector        
            scoped_ptr<iCmpExt::ICmpConfRtpPoint> m_rtpReceiverPoint;
            scoped_ptr<RtpSigDetector> m_detector;

            enum Stage{st_rtpSrc, st_localGenSrc};
            Stage m_stage;

            iCmpExt::CmpChannelId ChCtrlId() const  {return &m_lCmpIntf;}
            iCmpExt::CmpChannelId RtpSrcCtrlId() const  {return &m_rtpPoint;}
            iCmpExt::CmpChannelId RtpReceivCtrlId() const  {return &m_rtpReceiverPoint;}


            void ConnectAllPointToConf()
            {            
				m_conf->AddPoint(*m_tdmPoint, true, false);
				m_conf->AddPoint(*m_rtpReceiverPoint, false, true);

                if (m_rtpPoint != 0) m_conf->AddPoint(*m_rtpPoint, true, false);
                else if (m_genPoint != 0) m_conf->AddPoint(*m_genPoint, true, false);
                else ESS_HALT("Right signal source not exist");
            }

            // ------------------------------------------------------------------------------------

            void DiscAllPointFromConf()
            {   
                m_conf.reset();
                return;

				m_conf->RemovePoint(*m_tdmPoint);

                if (m_rtpReceiverPoint != 0) m_conf->RemovePoint(*m_rtpReceiverPoint);

                if (m_rtpPoint != 0) m_conf->RemovePoint(*m_rtpPoint);
                if (m_genPoint != 0) m_conf->RemovePoint(*m_genPoint);
            }        

            // ------------------------------------------------------------------------------------

            bool AllSourceReady() const
            {     
                bool rSrcReady = true;
                if (m_rtpPoint != 0) rSrcReady = (m_rtpPointSrc != 0);
                else ESS_ASSERT(m_genPoint != 0);

                return rSrcReady && m_rtpChannSrc != 0 && m_rtpReceiverPoint != 0;
            }

            // ------------------------------------------------------------------------------------

            void TryBeginDetect()
            {
                if (!AllSourceReady()) return;

                ConnectAllPointToConf();

                // start detection                                    
                m_rtpReceiverPoint->RtpSend( m_detector->LocalRtpAddr() );            
                m_detector->StartDetect();
            }

        // IDetectorEvents
        private:
            void DetectionFinish(bool ok, const std::string& descr)
            {
                m_owner.Finish(ok, descr);
            }

        // ICmpChannelEvent
        private:   
            void ChannelCreated(ICmpChannel *cmpChannel, CmpChannelId id)
            {
                ESS_ASSERT(ChCtrlId() == id);
                ESS_ASSERT(m_lCmpIntf == 0);

                m_lCmpIntf.reset(cmpChannel);

                // start            
                iMedia::Codec codec = cmpChannel->UseAlaw() ? iMedia::Codec::ALaw_8000 : iMedia::Codec::ULaw_8000;

                m_rtpChannSrc.reset(
                    new RtpSigSource(cmpChannel->ChannelRtpAddr(), codec, m_owner.getProfile().m_lFreq, CdBm0,
                    m_owner.getMsgThread(), m_owner.getLog(), m_rtpInfra->Infra() )
                    );

                m_lCmpIntf->StartRecv();     
            }

            // ------------------------------------------------------------------------------------

            void ChannelDeleted(CmpChannelId id, const std::string &desc)
            {
                ESS_ASSERT(ChCtrlId() == id);
                m_owner.Finish(false, desc);    
            }

            // ------------------------------------------------------------------------------------

            void DtmfEvent(const ICmpChannel *cmpChannel, const std::string &event)
            {
                ESS_UNIMPLEMENTED;
            }          

            // ------------------------------------------------------------------------------------

        // Create rtpPoint callback
        // iCmpExt::ICmpConfRtpPointEvents    
        private:

            void PointCreated(CmpConfRtpPointId rtpPointId, ICmpConfRtpPoint *pPoint)
            {
                ESS_ASSERT(RtpReceivCtrlId() == rtpPointId || RtpSrcCtrlId() == rtpPointId);

                if (RtpReceivCtrlId() == rtpPointId)
                {
                    ESS_ASSERT(m_rtpReceiverPoint == 0);
                    m_rtpReceiverPoint.reset(pPoint);
                }
                else if(RtpSrcCtrlId() == rtpPointId)
                {
                    ESS_ASSERT(m_rtpPoint == 0);
                    m_rtpPoint.reset(pPoint);
                    Utils::HostInf addr = m_rtpPoint->LocalRtp();            
                    iMedia::Codec codec = PayloadToCodec(m_owner.getProfile().m_srcPayload);

                    m_rtpPointSrc.reset(
                        new RtpSigSource(
                        addr, codec, m_owner.getProfile().m_rFreq, CdBm0,
                        m_owner.getThread(),  m_owner.getLog(), m_rtpInfra->Infra()
                        )
                        );                
                } 			

                TryBeginDetect();            
            }

            // ------------------------------------------------------------------------------------

            void PointDeleted(CmpConfRtpPointId rtpPointId, const std::string &desc)
            {
                ESS_ASSERT(RtpReceivCtrlId() == rtpPointId || RtpSrcCtrlId() == rtpPointId);
                m_owner.Finish(false, desc);
            }

            // ------------------------------------------------------------------------------------

            void PointDeleted(const ICmpConfPoint *p, const std::string &desc) // ICmpConfPointEvents 
            {
                ESS_ASSERT(m_conf.get() == p || m_tdmPoint.get() == p || m_genPoint.get() == p);
                m_owner.Finish(false, desc);
            }       

        public:

            TestImpl(
                NObjConfTester& owner,
                iCmpExt::ICmpChannelCreator& lIntf, iCmpExt::ICmpConfManager& rIntf,
                bool useRtp
                ) :
                m_owner(owner)            
              {
                  const ConfTestProfile& prof = m_owner.getProfile();

                  iRtp::RtpInfraParams par("127.0.0.1", prof.m_minRtpPort, prof.m_maxRtpPort);            
                  m_rtpInfra.reset( new iRtp::RtpPcInfra(m_owner.getThread(), par) );

                  //1 create signal source on left side            
                  lIntf.CreateCmpChannel(
                      ChCtrlId(), prof.m_channel,
                      m_selfWeakHost.Create<iCmpExt::ICmpChannelEvent&>(*this),
                      m_owner.getLog().LogCreatorWeakRef()
                      );

                  //2 create conference
                  m_conf.reset( rIntf.CreateCmpConf(*this, m_owner.getLog(), true) );

                  //3 create local souce in conf         
                  if (useRtp)
                  {                

                      rIntf.CreateRtpPoint(
                          RtpSrcCtrlId(), m_selfWeakHost.Create<iCmpExt::ICmpConfRtpPointEvents&>(*this),
                          m_owner.getLog().LogCreatorWeakRef(),
                          PayloadToCodecName(prof.m_srcPayload), ""
                          );
                  }
                  else
                  {
                      iDSP::Gen2xProfile genProf;
                      genProf.Freq0 = prof.m_rFreq;
                      genProf.Freq1 = 300;
                      genProf.Freq0Lev = CdBm0;
                      genProf.Freq1Lev = -40;
                      genProf.GenInterval = 200000;

                      m_genPoint.reset(rIntf.CreateGenPoint(*this, m_owner.getLog(), genProf));
                  }                        

                  //4 create tdm point            
                  m_tdmPoint.reset(rIntf.CreateTdmPoint(*this, m_owner.getLog(), prof.m_channel));            

                  //5 create receiver point                        
                  rIntf.CreateRtpPoint(
                      RtpReceivCtrlId(),
                      m_selfWeakHost.Create<iCmpExt::ICmpConfRtpPointEvents&>(*this),
                      m_owner.getLog().LogCreatorWeakRef(),
                      PayloadToCodecName(prof.m_receivePayload), ""
                      );

                  iMedia::Codec codec = PayloadToCodec(prof.m_receivePayload);
                  m_detector.reset(
                      new RtpSigDetector(*this, codec, prof.m_lFreq, prof.m_rFreq, CdBm0,
                      m_owner.getThread(),  m_owner.getLog(), m_rtpInfra->Infra()
                      ) 
                      );             
              }   

              void ResetConf()
              {
                  m_conf.reset();
              }

              ~TestImpl()
              {
                  DiscAllPointFromConf();
              }

        };

    } // namespace ConfTests
};

// ------------------------------------------------------------------------------------

namespace HiLevelTests
{
    namespace ConfTest
    {
        NObjConfTester::NObjConfTester( Domain::IDomain *pDomain, const Domain::ObjectName &name ):
            Domain::NamedObject(pDomain, name),
            m_timer(this, &NObjConfTester::OnTimer)
        {
        }

        // ------------------------------------------------------------------------------------

        NObjConfTester::~NObjConfTester()
        {}

        // ------------------------------------------------------------------------------------

        void NObjConfTester::Run(DRI::IAsyncCmd* pAsyncCmd, QString b1, QString b2, bool useRtp)
        {        
            AsyncBegin(pAsyncCmd);

            if (!useRtp)
            {
                int i = 0;  // debug
            }

            QString err;
            iCmpExt::ICmpChannelCreator *p1 = getDomain().FindFromRoot<iCmpExt::ICmpChannelCreator>(b1, &err);
            if (!p1) AsyncComplete(false, err);        

            iCmpExt::ICmpManager *p2 = getDomain().FindFromRoot<iCmpExt::ICmpManager>(b2, &err);
            if (!p2) AsyncComplete(false, err);        

            m_impl.reset(new TestImpl(*this, *p1, p2->ConfManager(), useRtp)) ;
            m_timer.Start(CTestTimeout);
        }        

        // ------------------------------------------------------------------------------------

        /*
        void NObjConfTester::Finish( bool ok, const std::string& descr )
        {           
            AsyncComplete(ok, descr.c_str());
            PutMsg(this, &NObjConfTester::OnReset);
        } */

        /*
        void NObjConfTester::Finish( bool ok, const std::string& desc )
        {                       
            PutMsg( this, &NObjConfTester::OnReset, FinishInfo(ok, desc) );
        }

        void NObjConfTester::OnReset(FinishInfo info)
        {
            m_impl.reset();

            iCore::MsgThreadFnDelayed(getMsgThread(), 
                boost::bind(&NObjConfTester::OnFinish, this, info), 
                500 );
        }

        void NObjConfTester::OnFinish( FinishInfo info )
        {
            AsyncComplete(info.Ok, info.Desc.c_str());
        } */

        void NObjConfTester::Finish( bool ok, const std::string& desc )
        {                       
            m_impl->ResetConf();

            iCore::MsgThreadFnDelayed(getMsgThread(), 
                boost::bind(&NObjConfTester::OnFinish, this, FinishInfo(ok, desc)), 
                500 );            
        }

        void NObjConfTester::OnFinish( FinishInfo info )
        {
            m_impl.reset();

            AsyncComplete(info.Ok, info.Desc.c_str());
        }

        // ------------------------------------------------------------------------------------

        void NObjConfTester::OnTimer( iCore::MsgTimer* )
        {
            // todo: m_impl->getInfo()
            Finish(false, "TestTimeout"); 
        }

    } // namespace ConfTest

} // namespace HiLevelTest





