#ifndef TDMBLOCKWRAPPERS_H
#define TDMBLOCKWRAPPERS_H

#include "BfTdm/IUserBlockInterface.h"
#include "TdmStreamStat.h"
#include "ITdmWriteWrapper.h"
#include "ITdmDataCapture.h"

#include "iDSP/g711codec.h"

namespace TdmMng
{
    using Platform::byte;

    class TdmEchoSuppress;
    
    // read-only operations for selected channel
    class TdmReadWrapper : boost::noncopyable
    {
        BfTdm::IUserBlockInterface &m_block;
        TdmProcessReadStatHelper *m_pHelper;
        TdmEchoSuppress &m_echo;
        const bool m_linearMode;

    public:

        TdmReadWrapper(BfTdm::IUserBlockInterface &block, 
                       TdmProcessReadStatHelper *pHelper,
                       TdmEchoSuppress &echo, bool linearMode) : // if linearMode do encode
        m_block(block), m_pHelper(pHelper), m_echo(echo), m_linearMode(linearMode)
        {
        }

        TdmProcessReadStatHelper* Helper()
        {
            return m_pHelper;
        }

        // methods for read
        // read all data associated with assigned timeSlot to buff,
        void Read(byte timeSlot, Utils::BidirBuffer& buff, ITdmDataCapture *pCapture = 0);

        int BlockSize() const
        {
            return m_block.GetTSBlockSize();
        }

        const Platform::word* GetFrame(int frameNum, int offset)
        {
            ESS_ASSERT(offset < 32);
            return m_block.GetRxBlock(frameNum) + offset;
        }

    };

    //-------------------------------------------------------------------------------------

    // write-only operations for selected channel
    class TdmWriteWrapper : public ITdmWriteWrapper, boost::noncopyable
    {
        BfTdm::IUserBlockInterface &m_block;      
        const iDSP::ICodec *m_pCodec;        

    // ITdmWriteWrapper impl
    public:

        void Write(byte timeSlot, word dstOffset, Platform::byte *pSrc, int srcCount);

        int BlockSize() const
        {
            return m_block.GetTSBlockSize();
        }

        Platform::word* GetFrame(int frameNum, int offset)
        {
            ESS_ASSERT(offset < 32);
            return m_block.GetTxBlock(frameNum) + offset;
        }

    public:

        TdmWriteWrapper(BfTdm::IUserBlockInterface &block, const iDSP::ICodec *pCodec) // if m_pCodec == 0 no decode
            : m_block(block), m_pCodec(pCodec)
        {}

        void CaptureData(ITdmDataCapture *pCapture)
        {
            if (pCapture != 0) pCapture->SetTxData(m_block);
        }

    };
} // namespace TdmMng

#endif
