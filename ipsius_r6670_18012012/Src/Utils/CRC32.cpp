#include "stdafx.h"

#include "CRC32.h"
#include "RawCRC32.h"

// ------------------------------------

namespace Utils
{
    
    void CRC32::TestAgainRaw()
    {
        QByteArray data;
        for(int i = 0; i < 32; ++i) data.push_back(i);

        Platform::dword rawCrc = Utils::UpdateCRC32(data.data(), data.size());
        Platform::dword boostCrc = Utils::CRC32(data).checksum();

        TUT_ASSERT(rawCrc == boostCrc);
    }

}  // namespace Utils

