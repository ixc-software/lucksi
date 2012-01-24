#include "stdafx.h"

#include "Utils/ErrorsSubsystem.h"

#include "TdmEchoSuppress.h"
#include "ChannelEcho.h"
#include "tdmdatacapture.h"
#include "TdmCodecs.h"

// ------------------------------------------------------

namespace TdmMng
{

    // debug -- don't use in release build!
    enum
    {
        CDropFirstSamplesToUart = 0, //8000 * 30,
    };

    // -------------------------------------------------------------------
            
    TdmEchoSuppress::TdmEchoSuppress( int maxChannels, const BfTdm::TdmProfile &tdmProfile, bool useAlaw )
    {
        {
            iDSP::ICodec *pCodec; 
            if (useAlaw) pCodec = new ACodec();
            else pCodec = new UCodec();
            m_codec.reset(pCodec);
        }

        for(int i = 0; i < tdmProfile.GetDMABlocksCount(); ++i)
        {
            TdmUserBlockExt *p = 
                new TdmUserBlockExt(maxChannels, tdmProfile.GetDMABlockCapacity(), *m_codec);
            m_extBlocks.Add(p);
        }
        m_extBlocksUsedCount = 0;

        // reserve echo
        {
            shared_ptr<ChannelEcho> pEmpty;
            for(int i = 0; i < maxChannels; ++i)
            {
                m_echo.push_back(pEmpty);
            }
        }
    }

    // --------------------------------------------------------------

    bool TdmEchoSuppress::ApplyEcho(byte chNumber, Utils::BidirBuffer& buff, TdmUserBlockExt *pExt,
        ITdmDataCapture *pCapture)
    {
        ESS_ASSERT(pExt != 0);
        ESS_ASSERT(chNumber < m_echo.size());

        shared_ptr<ChannelEcho> echo = m_echo.at(chNumber);
        if (echo == 0) return false;

        const std::vector<byte> *pSyncTx = pExt->SyncTx().GetTxBlock(chNumber);
        if (pSyncTx == 0) return false;

        return echo->ApplyEcho(buff, *pSyncTx, pCapture);
    }

    // --------------------------------------------------------------

    void TdmEchoSuppress::EnableForChannel( int chNumber, int taps )
    {
        ESS_ASSERT(chNumber < m_echo.size());

        ChannelEcho *pEcho = 
            new ChannelEcho(chNumber, taps, *m_codec, CDropFirstSamplesToUart);

        {
            iVDK::CriticalRegion cr;
            m_echo.at(chNumber).reset(pEcho);
        }
    }

    // --------------------------------------------------------------

    void TdmEchoSuppress::DisableForChannel( int chNumber )
    {
        ESS_ASSERT(chNumber < m_echo.size());

        {
            iVDK::CriticalRegion cr;
            m_echo.at(chNumber).reset();
        }
    }

    // --------------------------------------------------------------

    void TdmEchoSuppress::IrqEchoDataCopy( BfTdm::IUserBlockInterface &ch )
    {
        // подвязка блока расширенной информации (делается один раз для блока)
        if (ch.GetExtInfo() == 0)
        {
            ESS_ASSERT(m_extBlocksUsedCount < m_extBlocks.Size());
            ch.SetExtInfo( m_extBlocks[m_extBlocksUsedCount++] );
        }

        TdmUserBlockExt *pExt = ch.GetExtInfo();
        ESS_ASSERT(pExt != 0);

        // скопировать данные для канала, на котором активна эхокомпенскация
        for(int i = 0; i < m_echo.size(); ++i)
        {
            if (m_echo.at(i) != 0) 
            {
                pExt->SyncTx().IrqEchoDataCopy(i, ch);
            }
            else
            {
                pExt->SyncTx().ClearEchoData(i);
            }

        }
    }

    const iDSP::ICodec& TdmEchoSuppress::getCodec() const
    {
        return *m_codec;
    }

}  // namespace TdmMng

