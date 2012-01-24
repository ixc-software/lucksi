#include "stdafx.h"

#include "FreqRecv/TdmFreqRecvPool.h"
#include "FreqRecv/FreqRecvDtmf.h"
#include "FreqRecv/FreqRecvHelper.h"

#include "TdmChannel.h"
#include "SeqGenerator.h"
#include "TdmException.h"
#include "DataPacket.h"
#include "TdmFullGenerator.h"

namespace 
{
    enum
    {
        CActiveSocketPollIntervalMs = 15,
        CInactiveSocketPollIntervalMs = 1000,
    };

    // генераторы работают только до момента подачи внешней команды по генерации
    enum
    {
        CEnableGenToTDM   = false,
        CEnableGenFromTDM = false,

        CEnableToLineIdleGen = true,
    };

} // namespace 

namespace TdmMng
{    
    using iLogW::EndRecord;

    // -------------------------------------------------------------------------------------

    void TdmChannel::StartGen(bool toLine, const iDSP::Gen2xProfile &data)
    {
        // generator over recv
        if (m_recv.Enabled() && toLine)
        {
            if (m_log->LogActive())
            {
                *m_log << m_tagChannelControl << "StartGen: terGeneratorOverRecv" << EndRecord;
            }

            ESS_THROW_T(TdmException, terGeneratorOverRecv);
        }

        // validate
        if ( !TdmFullGenerator::Validate(data) )
        {
            if (m_log->LogActive())
            {
                *m_log << m_tagChannelControl << "StartGen: terBadGeneratorParams" << EndRecord;
            }

            ESS_THROW_T(TdmException, terBadGeneratorParams);
        }

        // add
        ITdmGenerator *p = new TdmFullGenerator(m_useAlaw, data, 0);

        scoped_ptr<ITdmGenerator> &gen = (toLine) ? m_genToTdm : m_genFromTdm;
        bool replaced = (gen.get() != 0);
        gen.reset( p );

        if (m_log->LogActive())
        {
            *m_log << m_tagChannelControl 
                << "StartGen:" 
                << ((replaced) ? " replaced" : " started")  
                << ((toLine) ? " to TDM" : " from TDM")  
                << EndRecord;
        }

    }

    // -------------------------------------------------------------------------------------

    void TdmChannel::StopGen(bool toLine)
    {
        if (toLine) m_genToTdm.reset(0);
              else  m_genFromTdm.reset(0);        
    }

    // -------------------------------------------------------------------------------------

    int TdmChannel::GetRtpPort() const
    {
        HostInf h = m_rtp.getLocalRtpAddr();
        return h.Port();
    }

    // -------------------------------------------------------------------------------------

    void TdmChannel::StartRecv()
    {    
        if (m_genToTdm.get() != 0)
        {
            if (m_log->LogActive())
            {
                *m_log << m_tagChannelControl << "StartRecv: stop generator" << EndRecord;
            }

            // destroy to line generator
            m_genToTdm.reset(0);
        }

        // start
        m_recv.Start();

        if (m_log->LogActive())
        {
            *m_log << m_tagChannelControl << "Start receive" << EndRecord;
        }
    }

    // -------------------------------------------------------------------------------------

    void TdmChannel::StopRecv()
    {                
        m_recv.Stop();

        if (m_log->LogActive())
        {
            *m_log << m_tagChannelControl << "Stop receive." << EndRecord;
        }
    }

    //-------------------------------------------------------------------------------------

    void TdmChannel::StartSend(const std::string &ip, int port)
    {   

		if (ip == "127.0.0.1")	ESS_THROW_T(TdmException, terSendToLocalhost);

        if (m_log->LogActive())
        {
            *m_log << m_tagChannelControl << "Start send." << EndRecord;                       
        }

        m_send.StartSend(ip, port, m_useAlaw);
    }

    // -------------------------------------------------------------------------------------

    void TdmChannel::StopSend( const std::string &ip, int port )
    {    
        if (m_log->LogActive())
        {
            *m_log  << m_tagChannelControl << "Stop send." << EndRecord;
        }
        
        m_send.StopSend(ip, port);
    }

    // ----------------------------------------------------------------------

    void TdmChannel::EchoSuppressMode( int taps )
    {
        if (taps > 0) 
        {
            m_mng.HAL().StartEchoSuppress(m_owner, m_number, taps);
        }
        else
        {
            m_mng.HAL().StopEchoSuppress(m_owner, m_number);
        }
    }

    // ----------------------------------------------------------------------

    void TdmChannel::SetFreqRecvMode( const std::string &recvName, const std::string &params, iCmp::FreqRecvMode mode )
    {
        m_freqRecvs->Command(recvName, mode, params);
    }

    // -------------------------------------------------------------------------------------

    void TdmChannel::DataCaptureModeMode( bool enabled )
    {
        const int CMinBuffSize = 160 * 3;

        TdmDataCapture *p = enabled ? new TdmDataCapture(m_mng, m_streamName, m_number, CMinBuffSize) : 0;
        m_dataCapture.reset(p);
    }

    // -------------------------------------------------------------------------------------
    

    TdmChannel::TdmChannel( ITdmOwner &owner, 
		Utils::SafeRef<iLogW::ILogSessionCreator> logCreator,		
		ITdmManager &mng, 
        int number, bool useAlaw, 
        const iDSP::ICodec &codec, const std::string &streamName) : 
        m_owner(owner),
        m_mng(mng), 
        m_streamName(streamName),
        m_number(number), 
		m_log(logCreator->CreateSession(
			std::string("Ch") + Utils::IntToString(m_number), true)),
		m_tagChannelControl(m_log->RegisterRecordKindStr("Control")),
		m_tagRWData(m_log->RegisterRecordKindStr("DataOp")),
        m_useAlaw(useAlaw),
        m_codec(codec),
        m_rtp(mng.getGeneralRtpParams(), *m_log, mng.getRtpInfra(), *this), 
        m_recv(mng, number, useAlaw),
        m_send(mng, m_rtp),
		m_eventSrc( iCmp::BfTdmEvent::MakeEventSource(streamName, number) )
    {        
        m_lastPollTime = 0;

        // generators
        if (CEnableGenFromTDM) m_genFromTdm.reset( new SeqGen1KHzALow() );
        if (CEnableGenToTDM)   m_genToTdm.reset( new SeqGen1KHzALow() );

        bool useGen = (CEnableGenFromTDM || CEnableGenToTDM);
        if ((useGen) && (m_log->LogActive()))
        {
            *m_log << "Warning! Generator(s) enabled!" << EndRecord;
        }

        // freq recievers
        m_freqRecvs.reset( new FreqRecv::FreqRecvPoolLocal(*this) );

    }

    // -------------------------------------------------------------------

	TdmChannel::~TdmChannel(){}

    // -------------------------------------------------------------------

    // no throw!
    void TdmChannel::FillReadBlock( TdmReadWrapper &block, Utils::BidirBuffer *pBuff )
    {
        ESS_ASSERT(pBuff != 0);

        try
        {
            if (m_genFromTdm != 0)
            {
                // fill block by gen
                m_genFromTdm->WriteTo(*pBuff, block.BlockSize());

                if (m_genFromTdm->Completed()) m_genFromTdm.reset(0);
            }
            else
            {
                // copy TDM data to buffer (with echo suppress if it enabled on this channel)
                block.Read(m_number, *pBuff, m_dataCapture.get()); 

                // is signal corrupted by echo?
                m_freqRecvs->ProcessData(*pBuff, false, m_streamName, m_number);  
            }
        }
        catch(/* const */ std::exception &e)
        {
            ESS_UNEXPECTED_EXCEPTION(e);
        }

    }

    // -------------------------------------------------------------------

    void TdmChannel::TdmRead( TdmReadWrapper &block )
    {   
        // nothing to do, just process FreqRecv
        if (m_send.Empty())
        {
            Utils::BidirBuffer *pBuff = m_freqRecvs->PeekDataBuffer();
            if (pBuff != 0)
            {
                block.Read(m_number, *pBuff); 
                m_freqRecvs->ProcessData(*pBuff, true, m_streamName, m_number);
            }
            return;
        }

        // TDM-in
        Utils::BidirBuffer* pBuff = CreateBidirBuffer();
        FillReadBlock(block, pBuff);  // no throw!

        DataPacket dataFromTdm(pBuff);  // RAII lock pBuff

        if (block.Helper() != 0) block.Helper()->BeginReadSend();  // fix stats

        // debug, push raw data to socket
        // m_rtp.RtpSockDirectWrite( dataFromTdm.DetachBuff() );

        // send data
        m_send.Send(dataFromTdm);

        if (block.Helper() != 0) block.Helper()->EndReadSend();
    }

    //-------------------------------------------------------------------------------------

    void TdmChannel::TdmWrite( TdmWriteWrapper &block )
    {        
        // генератор в линию имеет самый высокий приоритет
        if (m_genToTdm.get() != 0)
        {
            m_genToTdm->WriteTo(block, m_number);

            if (m_genToTdm->Completed()) m_genToTdm.reset(0);
            return;
        }

        // данные из RTP/TDM
        if ( m_recv.Enabled() )
        {
            ITdmSource &src = m_recv.GetTdmSource();

            // fill TDM from source
            if ( !src.WriteToBlock(block) )
            {
                m_mng.HAL().IdleGenerator(block, m_number);
            }

            // grab output data from TDM TX
            block.CaptureData( m_dataCapture.get() );

            return;
        }

        // idle generator
        if (CEnableToLineIdleGen)
        {
            m_mng.HAL().IdleGenerator(block, m_number);
        }
    }

    //-------------------------------------------------------------------------------------

    int TdmChannel::RtpPort() const
    {
        return GetRtpPort();
    }

    //-------------------------------------------------------------------------------------

    ITdmIn& TdmChannel::GetTdmInput()
    {
        return m_recv.GetTdmInput();
    }

    // -------------------------------------------------------------------------------------

    void TdmChannel::SocketPoll(int currTime, Utils::StatElementForInt &sockStat)
    {
        bool dropPack = true;
        int  interval = CInactiveSocketPollIntervalMs;

        if (m_recv.Enabled())
        {
            dropPack = false;
            interval = CActiveSocketPollIntervalMs;
        }

        if (interval < 0) return;
        if (currTime - m_lastPollTime < interval) return;

        m_rtp.SocketPoll(dropPack, sockStat);

        m_lastPollTime = currTime;
    }

    // ------------------------------------------------------------------

    bool TdmChannel::IsActive()
    {
        return ( m_recv.Enabled() ) || (!m_send.Empty());
    }

    // ------------------------------------------------------------------

    void TdmChannel::RxData( BidirBuffer* pBuff, iRtp::RtpHeaderForUser header )
    {
        // check state - ?
        // ... 

        m_recv.PutRtpData(pBuff, header);
    }

    // -----------------------------------------------------------------

    void TdmChannel::RxEvent( iRtp::RtpEvent ev, dword timestamp )
    {
        // if (m_state != st_inactive && m_statActive) ++m_lastStat.RtpEventControlCounter;
    }

    // -----------------------------------------------------------------

    void TdmChannel::RtpErrorInd( iRtp::RtpError error )
    {
        if (error.Kind() == iRtp::RtpError::SocketSendFail && !m_send.Paused())
        {
            m_send.PauseSend();
            m_mng.Queue().Push( 
                TdmAsyncEvent(m_eventSrc, iCmp::BfTdmEvent::CForcedStopSend()) 
                );
        }

        // error count - ?
        // ... 
    }

    // -----------------------------------------------------------------

    void TdmChannel::NewSsrcRegistred( Platform::dword ssrc )
    {
        // nothing
        // ... 
    }

    // -----------------------------------------------------------------

    void TdmChannel::RouteAozEvent(TdmMng::AozLineEvent e)
    {
        m_freqRecvs->RouteAozEvent(e);
    }

    // -----------------------------------------------------------------

    /*
    void TdmChannel::PushFreqRecvEvent( const std::string &recvName, const std::string &event )
    {
        using iCmp::BfTdmEvent;

        m_mng.PushAsyncEvent( BfTdmEvent::MakeFreqRecvEvent(m_eventSrc, recvName, event) );
    } */

    void TdmChannel::PushFreqRecvEvent( const std::string &devName, int chNum, const std::string &freqRecvName, const std::string &data )
    {
        typedef FreqRecv::FreqRecvHelper Helper;

        Helper::PushFreqRecvEvent(m_mng.Queue(), devName, chNum, freqRecvName, data);
    }

    void TdmChannel::PushDialBeginEvent( const std::string &devName, int chNum )
    {
        typedef FreqRecv::FreqRecvHelper Helper;

        Helper::PushDialBeginEvent(m_mng.Queue(), devName, chNum);
    }

    // -----------------------------------------------------------------

    FreqRecv::ITdmFreqRecvLocal* TdmChannel::CreateFreqRecv( const std::string &name, 
        const std::string &params )
    {
        return FreqRecv::FreqRecvPoolLocal::CreateLocalFreqRecv(name, params);
    }

    // -----------------------------------------------------------------

    Utils::BidirBuffer* TdmChannel::CreateBidirBuffer()
    {
        return m_mng.getRtpInfra().CreateBuff();
    }

} // namespace TdmMng

