#include "stdafx.h"

#include "ErrorsSubsystem.h"
#include "RawCRC32.h"

// -------------------------------------------

namespace
{
    using Platform::dword;
    using Platform::byte;

    class CrcTable
    {
        enum
        {
            CTableSize = 256,
        };

        dword m_tab[CTableSize];

        static dword Reflect(dword ref, byte ch) 
        {
            dword value = 0;

            // Swap bit 0 for bit 7 
            // bit 1 for bit 6, etc. 
            for(int i = 1; i < (ch + 1); i++) 
            { 
                if(ref & 1) 
                    value |= 1 << (ch - i); 
                ref >>= 1; 
            } 

            return value; 
        }

    public:

        /*
        CrcTable()
        {
            Platform::dword crc, poly;
            int i, j;

            poly = 0x04C11DB7; // 0xEDB88320L;
            for (i = 0; i < CTableSize; i++)
            {
                crc = i;
                for (j = 8; j > 0; j--)
                {
                    if (crc & 1)
                    {
                        crc = (crc >> 1) ^ poly;
                    }
                    else
                    {
                        crc >>= 1;
                    }
                }

                m_tab[i] = crc;
            }
        } */

        CrcTable()
        {
            dword ulPolynomial = 0x04c11db7;

            for(int i = 0; i < CTableSize; i++) 
            { 
                m_tab[i] = Reflect(i, 8) << 24; 

                for (int j = 0; j < 8; j++) 
                {
                    m_tab[i] = (m_tab[i] << 1) ^ (m_tab[i] & (1UL << 31) ? ulPolynomial : 0); 
                }

                m_tab[i] = Reflect(m_tab[i], 32); 
            } 
        }

        dword operator[](int indx) const
        {
            ESS_ASSERT(indx < CTableSize);  // paranoic
            return m_tab[indx];
        }

    };

    CrcTable GTable;

}

// -------------------------------------------

namespace Utils
{

    dword UpdateCRC32(const void *pBlock, int length, dword crc, bool finalize)
    {
        const byte *block = static_cast<const byte*>(pBlock);

        for (int i = 0; i < length; i++)
        {
            crc = ((crc >> 8) & 0x00FFFFFF) ^ GTable[(crc ^ *block++) & 0xFF];
        }

        if (finalize) crc ^= 0xFFFFFFFF;
        return crc;        
    }

    // ---------------------------------------------------

    void TestRawCrc32()
    {
        std::vector<byte> data;
        for(int i = 0; i < 32; ++i) data.push_back(i);

        dword rawCrc = UpdateCRC32(&data[0], data.size());
        TUT_ASSERT(rawCrc == 0x91267e8a);
    }

        
}  // namespace Utils

