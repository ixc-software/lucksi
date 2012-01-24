
#ifndef __SBPROTO_CRC__
#define __SBPROTO_CRC__

#include "SbpTypes.h"
#include "Utils/MemWriterStream.h"

namespace SBProto
{
    // CRC16
    class CRC
    {
        word m_crc;
        
    public:
        CRC(word crc);
        // get data between startPos and endPos and check CRC
        CRC(const byte *pData, size_t startPos, size_t size);
        CRC(boost::shared_ptr<Utils::ManagedMemBlock> data);
        CRC(const CRC &other) : m_crc(other.m_crc) {}

        CRC& operator=(const CRC &other) 
        {
            m_crc = other.m_crc;
            return *this;
        }
        
        bool operator==(const CRC &other) const;
        bool operator!=(const CRC &other) const;

        word get() const { return m_crc; }
    };
    
} // namespace SBProto

#endif
