
#include "stdafx.h"
#include "SbpUtils.h"
#include "Platform/Platform.h"

#include "Utils/ErrorsSubsystem.h"

namespace SBPUtils
{
    // DynamicLengthParser impl
    
    DynamicLengthParser::DynamicLengthParser() 
    : m_len(0), m_offset(0), m_complete(false), m_lengthSizeBytes(0)
    {
    }

    // -----------------------------------------------------------------------

    void DynamicLengthParser::Add(byte b)
    {
        ESS_ASSERT(!m_complete);
        
        ++m_lengthSizeBytes;
        
        byte getMask = 0x7f; // 01111111b
        dword tmp = (b & getMask);
        tmp <<= m_offset;
        
        m_len |= tmp;
        m_offset += 7;

        byte checkMask = 0x80; // 10000000b
        if ((b & checkMask) == 0) m_complete = true;
    }

    // -----------------------------------------------------------------------

    dword DynamicLengthParser::getLength() const 
    { 
        ESS_ASSERT(IsComplete());
        return m_len; 
    }

    // -----------------------------------------------------------------------

    size_t DynamicLengthParser::getLengthSizeBytes() const 
    {
        ESS_ASSERT(IsComplete()); 
        return m_lengthSizeBytes; 
    }

    // -----------------------------------------------------------------------

    void DynamicLengthParser::Reset() 
    {
        m_len = 0;
        m_offset = 0;
        m_complete = false;
        m_lengthSizeBytes = 0;
    }

    // -----------------------------------------------------------------------
    // Dynamic length writer
    /*
    void DynamicLengthWriter(Utils::BinaryWriter<Utils::MemWriterDynStream> &writer, 
                            dword len)
    {
        if (len == 0) 
        {
            byte byteLen = 0;
            // writer.WriteByte(byteLen);
            writer.Write(byteLen);
            return;
        }

        // get 8 bit, if it's not full length, change hi bit to 1, 
        // get next 8 bit from position (i + 7)
        dword checkLen = len;
        bool hiBitIsOne = false;
        for (size_t i = 0; i < sizeof(len); ++i)
        {
            byte currentByte = (len >> (i * 7)) & 0xff;
            byte hiBitMask = 0x80; // 10000000b

            if ((currentByte & hiBitMask) != 0) hiBitIsOne = true;

            dword writenByte = 0;
            writenByte |= ((len >> (i * 8)) & 0xff);
            writenByte <<= (i * 8);
            if (checkLen > 0) checkLen -= writenByte; // for numbers with hi bit == 1

            if ((checkLen == 0) && (!hiBitIsOne) && (currentByte != 0))
            {
                // writer.WriteByte(currentByte);
                writer.Write(currentByte);
                break;
            }

            // writer.WriteByte(currentByte | hiBitMask);
            writer.Write(currentByte | hiBitMask);
            hiBitIsOne = false;
        }
    }
    */

    /*
    // -----------------------------------------------------------------------
    // MemReaderExtendableStream impl

    void MemReaderExtendableStream::RecreateStream()
    {
        m_stream.reset();
        m_stream = boost::shared_ptr<Utils::MemReaderStream>(
                        new Utils::MemReaderStream(m_pBuffStartAddr, m_buffSize));
    }
    
    // -----------------------------------------------------------------------

    void MemReaderExtendableStream::SetBuffer(void *pStartAddr, size_t size)
    {
        ESS_ASSERT(pStartAddr != 0);
        ESS_ASSERT(size != 0);
        
        m_pBuffStartAddr = pStartAddr;
        m_buffSize = size;
        m_absoluteReadPos = 0;
        RecreateStream();
    }

    // -----------------------------------------------------------------------

    void MemReaderExtendableStream::ResetBuffer()
    {
        m_stream.reset();
        m_pBuffStartAddr = 0;
        m_buffSize = 0;
        m_absoluteReadPos = 0;
    }

    // -----------------------------------------------------------------------

    void MemReaderExtendableStream::ExtendBuffer(size_t size) 
    {
        ESS_ASSERT(m_stream.get() != 0);
        m_buffSize += size;
        RecreateStream();
    }

    // -----------------------------------------------------------------------

    byte MemReaderExtendableStream::ReadByte()    // can throw
    {
        ESS_ASSERT(m_stream.get() != 0);
        m_absoluteReadPos += sizeof(byte);
        
        return m_stream->ReadByte(); 
    }

    // -----------------------------------------------------------------------
    
    void MemReaderExtendableStream::Read(void *pDest, size_t length) // can throw
    {
        ESS_ASSERT(m_stream.get() != 0);
        m_absoluteReadPos += length;
        
        m_stream->Read(pDest, length);
    }
    */
    // -----------------------------------------------------------------------

    /*
    // BiDirMemWriterDynStream impl

    BiDirMemWriterDynStream::BiDirMemWriterDynStream(size_t offsetFromBegin)
    : m_buff(new Utils::DynamicMemBlock(offsetFromBegin)), m_toFinishPos(offsetFromBegin), 
        m_toStartPos(offsetFromBegin - 1), m_isWritingToFinish(true)
    {
        ESS_ASSERT(offsetFromBegin != 0);
    }

    // -----------------------------------------------------------------------

    size_t BiDirMemWriterDynStream::CheckSizeAndGetWritePos(size_t size)
    {
        if (!m_isWritingToFinish)
        {
            // ESS_ASSERT((m_toStartPos - size) >= 0);
            ESS_ASSERT(size <= (m_toStartPos + 1));
            
            return (m_toStartPos - size);
        }
        
        ESS_ASSERT((m_toFinishPos + size) <= m_buff->Size());

        size_t sizeForResize = ((m_toFinishPos + size) <= m_buff->Size())? 
                                0 : (m_toFinishPos + size - m_buff->Size());
        m_buff->Resize (sizeForResize);
        
        return m_toFinishPos;
    }

    // -----------------------------------------------------------------------

    void BiDirMemWriterDynStream::ChangeWritePos(size_t size)
    {
        if (m_isWritingToFinish) m_toFinishPos += size;
        else m_toStartPos -= size;
    }

    // -----------------------------------------------------------------------

    void BiDirMemWriterDynStream::WriteByte(byte b)
    {
        size_t writePos = CheckSizeAndGetWritePos(sizeof(b));
        
        *(m_buff->getData() + writePos) = b;

        ChangeWritePos(sizeof(b));
    }

    // -----------------------------------------------------------------------
    
    void BiDirMemWriterDynStream::Write(void *pSrc, size_t length)
    {
        size_t writePos = CheckSizeAndGetWritePos(length);

        memcpy((m_buff->getData() + writePos), pSrc, length);

        ChangeWritePos(length);
    }

    // -----------------------------------------------------------------------

    boost::shared_ptr<Utils::DynamicMemBlock> BiDirMemWriterDynStream::GetData() const
    {
        return GetDataFromPos(m_toStartPos);
    }

    // -----------------------------------------------------------------------
    
    boost::shared_ptr<Utils::DynamicMemBlock> 
        BiDirMemWriterDynStream::GetDataFromPos(size_t pos) const
    {
        ESS_ASSERT((pos < m_toFinishPos) && (pos >= m_toStartPos));
        size_t dataSize = m_toFinishPos - pos;

        Utils::MemWriterDynStream res(dataSize, false);
        Utils::BinaryWriter<Utils::MemWriterDynStream> writer(res);
        writer.Write((m_buff->getData() + pos), dataSize);

        return res.DetachBuffer();
        
    }
    */
    

} // namespace SBPUtils
