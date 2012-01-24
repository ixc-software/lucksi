
#include "stdafx.h"
#include "SbpCRC.h"
#include "Utils/ErrorsSubsystem.h"


namespace SBProto
{
    // CRC impl
    
    CRC::CRC(word crc) : m_crc(crc) 
    {
    }

    // -------------------------------------------------------

    CRC::CRC(const byte *pData, size_t startPos, size_t size) : m_crc(0)
    {
        size_t endPos = startPos + size;
        for (size_t i = startPos; i < endPos; ++i)
        {
            m_crc += *(pData + i);
        }
    }

    // -------------------------------------------------------

    CRC::CRC(boost::shared_ptr<Utils::ManagedMemBlock> data): m_crc(0)
    {
        for (size_t i = 0; i < data->Size(); ++i)
        {
            m_crc += *(data->getData() + i);
        }
    }

    // -------------------------------------------------------

    bool CRC::operator==(const CRC &other) const 
    { 
        return (m_crc == other.m_crc); 
    }

    // -------------------------------------------------------
    
    bool CRC::operator!=(const CRC &other) const 
    { 
        return !(m_crc == other.m_crc); 
    }
    
} // namespace SBProto
