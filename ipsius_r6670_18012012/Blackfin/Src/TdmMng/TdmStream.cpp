#include "stdafx.h"
#include "Platform/Platform.h"
#include "TdmStream.h"

// ----------------------------------------------------------

namespace
{
    using namespace TdmMng;

    enum
    {
        CNoIrqProcess = false,
    };

}  // namespace

// ----------------------------------------------------------

namespace TdmMng
{
    void TdmStream::DoSimpleLog( const iLogCommon::LogString& msg, const LogRecordTag& tag )
    {
        if (m_log->LogActive()) m_log->Add(msg, tag);
    }

    //-----------------------------------------------------------------------------------------

    void TdmStream::TdmFrameRead( TdmReadWrapper &block )
    {
        for(int i = 0; i < m_channels.Size(); ++i)
        {
            m_channels[i]->TdmRead(block);
        }
    }

    //-------------------------------------------------------------------------------------

    void TdmStream::TdmFrameWrite( TdmWriteWrapper &block )
    {
        for(int i = 0; i < m_channels.Size(); ++i)
        {
            m_channels[i]->TdmWrite(block);
        }
    }

    //-------------------------------------------------------------------------------------

    TdmStream::TdmStream(ITdmManager &mng,
		Utils::SafeRef<iLogW::ILogSessionCreator> logCreator,
		const std::string &name,
        const TdmChannelsSet &channels, int sportNum, int sportChannel, bool useAlaw) : 
        m_mng(mng),
        m_name(name),
        m_useAlaw(useAlaw),
        m_codec( TdmCodec::CreateCodec(useAlaw) ),
        m_pTdm(0),
        m_timer( BfDev::BfTimerCounter::GetFreeTimer() )
    {
        m_log.reset(logCreator->CreateSession(name, true));
        m_logTag = m_log->RegisterRecordKindStr("stream");

		if (m_log->LogActive())
        {
            *m_log << m_logTag << "Started!" << EndRecord;
        }
            

        // fill m_channels by channels set
        int max = channels.MaxValue();

        for(int i = 0; i <= max; ++i)
        {
            if (channels.IsExist(i))
            {
                TdmChannel *pCh = 
                    new TdmChannel(*this, logCreator, m_mng, i, useAlaw, *m_codec, name);

                m_channels.Add(pCh);
            }                
        }
        
        // register in HAL
        m_pTdm = m_mng.HAL().RegisterTdm(sportNum, sportChannel, *this, useAlaw);        
        ESS_ASSERT(m_pTdm != 0);
    }

    //-------------------------------------------------------------------------------------

    TdmStream::~TdmStream()
    {
        // unregister from HAL
        m_mng.HAL().UnregTdm(*this);
    }

    //-------------------------------------------------------------------------------------

    const std::string &TdmStream::Name() const
    {
        return m_name;
    }

    //-------------------------------------------------------------------------------------

    ITdmChannel* TdmStream::Channel( int number )
    {
        for(int i = 0; i < m_channels.Size(); ++i)
        {
            if (m_channels[i]->Number() == number) return m_channels[i];
        }

        return 0;
    }

    //-------------------------------------------------------------------------------------

    TdmChannel* TdmStream::FindChannelByRtpPort( int rtpPort )
    {
        for(int i = 0; i < m_channels.Size(); ++i)
        {
            if (m_channels[i]->RtpPort() == rtpPort) return m_channels[i];
        }

        return 0;
    }

    // -------------------------------------------------------------------------------------

    void TdmStream::SocketPoll(Utils::StatElementForInt &sockStat)
    {
        int currTime = Platform::GetSystemTickCount();

        for(int i = 0; i < m_channels.Size(); ++i)
        {
            m_channels[i]->SocketPoll(currTime, sockStat);
        }        
    }

    // -------------------------------------------------------------------------------------

    void TdmStream::FillRtpPorts( std::vector<int> &ports ) const
    {
        ports.clear();

        for(int i = 0; i < m_channels.Size(); ++i)
        {
            ITdmChannel *pCh = m_channels[i];
            ports.push_back( pCh->GetRtpPort() );            
        }        
    }

    // -------------------------------------------------------------------------------------

    int TdmStream::ActiveChannelsCount() const
    {
        int count = 0;

        for(int i = 0; i < m_channels.Size(); ++i)
        {
            if (m_channels[i]->IsActive()) count++;
        }        

        return count;
    }

    // -------------------------------------------------------------------------------------

    void TdmStream::BlockProcess(ITdmBlock &block)
    {
        if (CNoIrqProcess) return;

        m_timer.Reset();

        dword tStart = CurrTick();            

        // 1. read data from TDM            
        {
            TdmProcessReadStatHelper helper(m_timer);

            dword tStart = CurrTick();

            block.ProcessReadFor(*this, &helper);

            m_stat.ReadProcessingTimeSend.Add( helper.ReadSendStageSum() );
            m_stat.ReadProcessingTimeEcho.Add( helper.EchoStageSum() );
            m_stat.TotalEchoBlocksCount += helper.EchoBlocks();

            m_stat.ReadProcessingTime.Add( CurrTick() - tStart );   // total stage time
        }

        // 2. read data from all channels socks (polling)
        {
            dword tStart = CurrTick();
            SocketPoll(m_stat.SocketDataIn);
            m_stat.SocketPollingTime.Add( CurrTick() - tStart );
        }


        // 3. write to TDM
        {
            dword tStart = CurrTick();
            block.ProcessWriteFor(*this);
            m_stat.WriteProcessingTime.Add( CurrTick() - tStart );
        }

        // something else - ?!
        // ...

        m_stat.FullIRQProcessingTime.Add( CurrTick() - tStart );
    }

    void TdmStream::RouteAozEvent( int chNum, TdmMng::AozLineEvent e )
    {
        bool toAny = (chNum < 0);

        for(int i = 0; i < m_channels.Size(); ++i)
        {
            ITdmChannel *pCh = m_channels[i];

            if (toAny || (pCh->GetChNumber() == chNum) )
            {
                pCh->RouteAozEvent(e);
            }
        }        
    }

} // namespace TdmMng

