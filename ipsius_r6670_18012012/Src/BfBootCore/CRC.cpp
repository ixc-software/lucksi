#include "stdafx.h"
#include "CRC.h"
#include "Utils/RawCRC32.h"

namespace BfBootCore
{   
    void CRC::ProcessBlock( const std::vector<byte>& block )
    {
        ProcessBlock(&block[0], block.size());
    }

    void CRC::ProcessBlock( const void* block, int size )
    {
        Utils::UpdateCRC32(block, size, m_crc, false);
    }
    CRC::CRC()
    {
        Reset();
    }

    BfBootCore::dword CRC::Release() /* подсчет CRC закончен, получить результат. */
    {
        m_crc = Utils::UpdateCRC32(0, 0, m_crc, true);
        dword crc = m_crc;
        Reset();
        return crc;
    }

    void CRC::Reset()
    {
        m_crc = Utils::RawCRC32::InitialValue;
    }
} // namespace BfBootCore

