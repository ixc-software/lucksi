#include "stdafx.h"
#include "TdmBlockWrappers.h"
#include "TdmEchoSuppress.h"

#include "iDSP/g711codec.h"

// ----------------------------------------------------------

namespace TdmMng
{
    using Platform::byte;
    using Platform::word;
    using Platform::int16;

    // -------------------------------------------------------------------

    void TdmReadWrapper::Read(byte timeSlot, Utils::BidirBuffer& buff, ITdmDataCapture *pCapture)
    {
        int blockSize = m_block.GetTSBlockSize();

        ESS_ASSERT(buff.Size() == 0);    
        ESS_ASSERT(blockSize > 0);    
        ESS_ASSERT(blockSize <= buff.getBlockSize());            

        if (!m_linearMode)
        {
            buff.SetDataOffset(buff.GetDataOffset(), blockSize); // set size
            m_block.CopyRxTSBlockTo(timeSlot, buff.Front(), 0, blockSize);
        }
        else
        {
            const iDSP::ICodec& codec = m_echo.getCodec();

            for (int i = 0; i < blockSize; ++i)
            {
                word sample;
                m_block.CopyRxTSBlockTo(timeSlot, &sample, i, 1);                            
                buff.PushBack( codec.Encode( (int16)sample ) );// translate word-->int & encode
            }            
        }

        if (m_pHelper != 0) m_pHelper->BeginEcho();
        if ( m_echo.ApplyEcho(timeSlot, buff, m_block.GetExtInfo(), pCapture) )
        {
            if (m_pHelper != 0) m_pHelper->EndEcho();
        }
        else
        {
            if (pCapture != 0) pCapture->SetRxData(buff);
        }
    }

    // -------------------------------------------------------------------

    void TdmWriteWrapper::Write(byte timeSlot, word dstOffset, Platform::byte *pSrc, int srcCount)
    {
        if (m_pCodec == 0)
        {
            m_block.PutTxTSBlockFrom(timeSlot, pSrc, dstOffset, srcCount);           
            return;
        }

        // decode               
        for (int i = 0; i < srcCount; ++i)
        {            
            word sample = (word)m_pCodec->Decode(pSrc[i]); // decode & translate int-->word
            m_block.PutTxTSBlockFrom(timeSlot, &sample, dstOffset + i, 1);   
        }
    }

}  // namespace TdmMng

