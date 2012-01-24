#include "stdafx.h"

#include "Utils/ErrorsSubsystem.h"
#include "Utils/IntToString.h"
#include "BfDev/BfUartSimpleCore.h"
#include "BfDev/SysProperties.h"

#include "tdmsynctxcopy.h"

namespace TdmMng
{
    using Platform::byte;
    using Platform::int16;
    using Platform::dword;
    using std::vector;
    
    class TdmSyncTxCopy::ChannelData : boost::noncopyable
    {
        int m_number;
        vector<byte> m_buff;  // sync TX data
        bool m_dataAvail;

    public:

        ChannelData(int number, int buffSize) : 
          m_number(number), m_buff(buffSize, 0), m_dataAvail(false)
        {
        }

        void Copy(BfTdm::IUserBlockInterface &block)
        {
            ESS_ASSERT(block.GetTSBlockSize() == m_buff.size());
            block.CopySyncTxTo(m_number, &m_buff[0], 0, m_buff.size());
            m_dataAvail = true;
        }

        void Clear()
        {
            m_dataAvail = false;
        }

        const std::vector<byte>* GetTxBlock() const
        {
            if (!m_dataAvail) return 0;

            return &m_buff;
        }


        /*
        bool AppyEcho(Utils::BidirBuffer &buff, Echo::EchoCancel &echo, iDSP::ICodec &codec)
        {
            if (!m_dataAvail) return false;

            ESS_ASSERT(buff.Size() == m_buff.size());

            const bool debugTrace = (CDropFirstSamplesToUart > 0);

            if (debugTrace) 
            {
                if (m_debug == 0) m_debug.reset( new DebugInfo(m_number) );
            }

            for(int i = 0; i < m_buff.size(); ++i)
            {
                int tx = codec.Decode( m_buff.at(i) );
                int rx = codec.Decode( buff.At(i) );

                int fixedRx = echo.Process(tx, rx);

                buff.At(i) = codec.Encode(fixedRx);

                if (debugTrace)   
                {
                    m_debug->Trace(tx, rx, fixedRx);
                }
            }

            ++m_blockCounter;

            if (debugTrace)
            {
                m_debug->TryFinalize();
            }

            return true;
        } */

    };

    
}  // namespace TdmMng

// ---------------------------------------------------------

namespace TdmMng
{

    TdmSyncTxCopy::TdmSyncTxCopy( int channelsCount, int channelBuffSize, 
        iDSP::ICodec &codec ) : m_codec(codec)
    {
        for(int i = 0; i < channelsCount; ++i)
        {
            m_channels.Add( new ChannelData(i, channelBuffSize) );
        }
    }

    TdmSyncTxCopy::~TdmSyncTxCopy()
    {
    }

    void TdmSyncTxCopy::IrqEchoDataCopy( int chNum, BfTdm::IUserBlockInterface &block )
    {
        ESS_ASSERT(chNum < m_channels.Size());
        m_channels[chNum]->Copy(block);
    }

    void TdmSyncTxCopy::ClearEchoData( int chNum )
    {
        ESS_ASSERT(chNum < m_channels.Size());
        m_channels[chNum]->Clear();
    }

    /*
    bool TdmSyncTxCopy::AppyEcho( byte chNum, Utils::BidirBuffer &buff, Echo::EchoCancel &echo )
    {
        ESS_ASSERT(chNum < m_channels.Size());
        return m_channels[chNum]->AppyEcho(buff, echo, m_codec);
    } */

    const std::vector<byte>* TdmSyncTxCopy::GetTxBlock( int chNum )
    {
        ESS_ASSERT(chNum < m_channels.Size());
        return m_channels[chNum]->GetTxBlock();
    }

   
}  // namespace TdmMng


