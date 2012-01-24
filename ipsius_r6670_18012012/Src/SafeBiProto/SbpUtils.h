
#ifndef __SBPUTILS__
#define __SBPUTILS__

#include "SbpTypes.h"
// #include "Utils/IBiRWStream.h"
#include "Utils/BinaryWriter.h"
// #include "Utils/MemWriterStream.h"
// #include "Utils/MemReaderStream.h"


namespace SBPUtils
{
    using namespace SBProto; 

    // Parse dynamic length byte by byte
    class DynamicLengthParser
    {
        dword m_len;
        byte m_offset;
        bool m_complete;
        size_t m_lengthSizeBytes;
        
    public:
        DynamicLengthParser();

        // Add bytes one by one until IsComplete() == true, 
        void  Add(byte b);
        void Reset();
        
        dword getLength() const;
        size_t getLengthSizeBytes() const;
        bool IsComplete() const { return m_complete; }
    };

    // ----------------------------------------------------------

    /*
    void DynamicLengthWriter(Utils::BinaryWriter<Utils::MemWriterDynStream> &writer, 
                             dword len);
    */

    // Make dynamic length and write it in given buffer
    template<class TWriterStream>
    void DynamicLengthWriter(Utils::BinaryWriter<TWriterStream> &writer, dword len)
    {
        if (len == 0) 
        {
            byte byteLen = 0;
            writer.WriteByte(byteLen);
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
                writer.WriteByte(currentByte);
                break;
            }

            writer.WriteByte(currentByte | hiBitMask);
            hiBitIsOne = false;
        }
    }

    /*
    // ---------------------------------------------------
    // Class provided to work in pair with Utils::MemWriterDynStream
    // ExtendBuffer() means that next data will be read from memory block 
    // after previous memory block.
    // Data readed from absolute reading position: 
    // position_before_ExtendBuffer == position_after_ExtendBuffer 
    // 
    // Class has the same interface as IBiReaderStream
    class MemReaderExtendableStream
    {
        boost::shared_ptr<Utils::MemReaderStream> m_stream;

        void *m_pBuffStartAddr;
        size_t m_buffSize;
        size_t m_absoluteReadPos;

        void RecreateStream();
        
    public:
        MemReaderExtendableStream() 
        : m_pBuffStartAddr(0), m_buffSize(0), m_absoluteReadPos(0)
        {
        }

        // Destroy previous buffer and create new one 
        void SetBuffer(void *pStartAddr, size_t size);
        
        // Destroy buffer
        void ResetBuffer();

        // Extend available size to read
        void ExtendBuffer(size_t size) ;

        size_t HasToRead() const { return  (m_buffSize - m_absoluteReadPos); }

    // IBiReaderStream semantic
    public:
        byte ReadByte();    // can throw
        void Read(void *pDest, size_t length); // can throw
    };
    */

    // -----------------------------------------------------------

    /*
    // Write data from the offsetFromBegin to the end or 
    // from the (offsetFromBegin - 1) to the begin.
    // Do not reverse writing string, when writing to the begin.
    // ..............|centre|................
    // Write: "abc", then 'd' to the begin and "ef", then 'g' to the end.
    // ..........dabc|centre|efg............
    // Default directin - direction to the end.
    // Size of data can be written after offsetFromBegin position is dynamic.
    class BiDirMemWriterDynStream
    {
        boost::shared_ptr<Utils::DynamicMemBlock> m_buff;

        size_t m_toStartPos;
        size_t m_toFinishPos;

        bool m_isWritingToFinish;

        size_t CheckSizeAndGetWritePos(size_t size);
        void ChangeWritePos(size_t size);

    public:
        BiDirMemWriterDynStream(size_t offsetFromBegin);

        // copy and return data from data (!buffer) start pos
        boost::shared_ptr<Utils::DynamicMemBlock> GetData() const;
        boost::shared_ptr<Utils::DynamicMemBlock> GetDataFromPos(size_t pos) const;
        size_t GetDataSize() const { return (m_toFinishPos - m_toStartPos); }
        
        void setIsWrittingToFinish() { m_isWritingToFinish  = true; }
        void setIsWrittingToStart() { m_isWritingToFinish = false; }

    // IBiWriterStream semantic
    public:
        void WriteByte(byte b);
        void Write(void *pSrc, size_t length);
    };
    */
 
} // namespace SBPUtils

#endif



