#include "stdafx.h"

#include "SndMix/ConferenceMng.h"

#include "TdmManager.h"
#include "TdmChannel.h"

// --------------------------------------------------

namespace TdmMng
{

    TdmManager::TdmManager( const TdmManagerProfile &profile ) : 
		MsgObject(profile.Thread),
        m_profileVerify(profile),
        m_prof(profile),
        m_log(profile.LogCreator->CreateSession("TdmMng", true)),
		m_tagUserCmd(m_log->RegisterRecordKindStr("UserCmd")),
		m_tagError(m_log->RegisterRecordKindStr("Error")),
        m_buffCreator(profile.BidirBuffSize, profile.BidirBuffCount, profile.BidirBuffOffset),
        m_rtpInfra(profile.Thread, profile.RtpInfraPar, m_buffCreator),
        m_localIp(profile.LocalIp),
        m_eventQueue(iCmp::BfTdmEvent::SrcQueueTdm())
    {
        HAL().RegisterEventsHandler(this);

        // conference
        m_conference.reset( 
            new SndMix::ConferenceMng(*m_log, 
                          m_eventQueue, m_generalRtpParams, m_rtpInfra, *this) 
            );

        // log
		if(m_log->LogActive())
		{
			*m_log << m_tagUserCmd << "TdmManager created. " << profile.getAsString() << EndRecord;
		}
    }

    // -------------------------------------------------------------------------------------

    TdmManager::~TdmManager()
    {
        HAL().UnregisterEventsHandler(this);
        HAL().StopDevices();  // here - ?

        m_conference.reset();
        
        m_streams.Clear();
    }

    //-------------------------------------------------------------------------------------

    TdmChannel* TdmManager::FindChannelByRtpPort( int rtpPort )
    {
        for(int i = 0; i < m_streams.Size(); ++i)
        {
            TdmChannel *pCh = m_streams[i]->FindChannelByRtpPort(rtpPort);
            if (pCh != 0) return pCh;
        }

        return 0;
    }

    //-------------------------------------------------------------------------------------

    bool TdmManager::IpIsLocal( const std::string &ip ) const
    {
        return m_localIp == ip;
    }

    //-------------------------------------------------------------------------------------

    iRtp::IRtpInfra& TdmManager::getRtpInfra()
    {
        return m_rtpInfra;
    }

    //-------------------------------------------------------------------------------------

    const iRtp::RtpParams& TdmManager::getGeneralRtpParams() const
    {
        return m_generalRtpParams;
    }

    //-------------------------------------------------------------------------------------

    const RtpRecvBufferProfile& TdmManager::getBufferingProf() const
    {
        return m_prof.BufferingProf;
    }

    //-------------------------------------------------------------------------------------

    TdmStream* TdmManager::RegisterStream(const std::string &name, 
        const TdmChannelsSet &channels, int sportNum, int sportChannel, bool useAlaw)
    {        
        if(m_log->LogActive())
		{
			*m_log << m_tagUserCmd
				<< "Register new stream with name " << name << ". Sport="
				<< sportNum << ", ch=" << sportChannel << iLogW::EndRecord;
		}

        // check for uniq name
        for (int i = 0; i < m_streams.Size(); ++i)
        {
            if ( m_streams[i]->Name() == name )
            {                
                if(m_log->LogActive()) *m_log << m_tagError << "Duplicate stream name!" << EndRecord;
                ESS_THROW_T(TdmException, TdmErrorInfo(terDuplicateNameStream));
            }
        }     

        TdmStream *pS = new TdmStream(*this, m_log->LogCreator(), name, channels, sportNum, sportChannel, useAlaw);

        m_streams.Add(pS);

        return pS;
    }

    //-------------------------------------------------------------------------------------

    TdmStream* TdmManager::Stream(const std::string &name, bool throwOnNull)
    {        
        for(int i = 0; i < m_streams.Size(); ++i)
        {
            if (m_streams[i]->Name() == name) return m_streams[i];
        }

        if (throwOnNull)
        {
            ESS_THROW_T(TdmMng::TdmException, TdmErrorInfo(terBadDeviceName, name));
        }

        return 0;
    }

    // ------------------------------------------------------------

    void TdmManager::OnTdmHalIrq(int sportNum, ITdmBlock &block)
    {
        for(int i = 0; i < m_streams.Size(); ++i)
        {
            m_streams[i]->BlockProcess(block);
        }

        // process only for voice SPORT
        if (sportNum == 0)
        {
            m_conference->Process();
        }
    }

    // ------------------------------------------------------------

    void TdmManager::OnTdmHalBlockCollision()
    {
        bool first = false;

        for(int i = 0; i < m_streams.Size(); ++i)
        {
            m_streams[i]->BlockCollision();
            first = (m_streams[i]->Stat().TdmBlockCollision == 1);
        }

        if (first)
        {
            if(m_log->LogActive()) *m_log << "First TDM block collision!!" << iLogW::EndRecord;
        }
    }

    // ------------------------------------------------------------

    /*
    void TdmManager::PushAsyncEvent( const TdmAsyncEvent &event )
    {
        m_eventQueue.Push(event);
    }

    void TdmManager::PushAsyncEvent( const TdmAsyncCaptureData &event )
    {
        m_eventQueue.Push(event);
    } */

    // ------------------------------------------------------------

    void TdmManager::SendAllEventsFromQueue(SBProto::ISafeBiProtoForSendPack &proto,
                                            TdmMng::IIEventsQueueItemDispatch &hook)
    {
        m_eventQueue.SendAll(proto, &hook);
    }

    // ------------------------------------------------------------

    void TdmManager::GetDeviceInfo( const std::string &streamName, /* out */ int &tdmBlockSize, bool &aLow )
    {
        // get data
        TdmStream *pS = Stream(streamName, true);
        const BfTdm::TdmProfile &prof = pS->Tdm().Profile();
        if (prof.GetCompandingLaw() != BfTdm::CNoCompanding)
        {
            ESS_THROW_T(TdmMng::TdmException, terTdmCompanding);
        }

        // set result 
        tdmBlockSize = prof.GetDMABlockCapacity();
        aLow = pS->UseAlaw();
    }

    // ------------------------------------------------------------

    void TdmManager::BindToConference( const std::string &streamName, int chNum, const Utils::SafeRef<TdmMng::ISendSimple> &send, const Utils::SafeRef<TdmMng::IRtpLikeTdmSource> &recv )
    {
        ITdmChannel &ch = Channel(streamName, chNum);
        ch.BindToConference(send, recv);
    }

    // ------------------------------------------------------------

    void TdmManager::UnbindFromConference( const std::string &streamName, int chNum, const Utils::SafeRef<TdmMng::ISendSimple> &send, const Utils::SafeRef<TdmMng::IRtpLikeTdmSource> &recv )
    {
        ITdmChannel &ch = Channel(streamName, chNum);
        ch.UnbindToConference(send, recv);
    }

    // ------------------------------------------------------------

    ITdmChannel& TdmManager::Channel( const std::string &streamName, int chNumber )
    {
        TdmStream *pS = Stream(streamName, true);
        ESS_ASSERT(pS != 0);

        ITdmChannel *pC = pS->Channel(chNumber);
        if (pC == 0)
        {
            std::ostringstream oss;
            oss << streamName << ":" << chNumber;
            ESS_THROW_T( TdmMng::TdmException, TdmErrorInfo(terBadDeviceChannel, oss.str()) ); 
        }

        return *pC;
    }

}  // namespace TdmMng
