#ifndef CRC_H
#define CRC_H

#include "Platform/PlatformTypes.h"

namespace BfBootCore
{    
    // Utils?

    using Platform::dword;
    using Platform::byte;    

    class CRC
    {
        dword m_crc;
        
    public:
        CRC();

        // ������ � �������� CRC ��������� ����. ����������� ������ �� ������ ���������.
        void ProcessBlock(const std::vector<byte>& block);
        void ProcessBlock(const void* block, int size);
        dword Release(); // ������� CRC ��������, �������� ���������.
        void Reset();

    };
} // namespace BfBootCore

#endif
