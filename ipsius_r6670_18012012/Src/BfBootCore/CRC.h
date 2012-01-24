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

        // учесть в рассчете CRC очередной блок. Очередность блоков не должна нарушатся.
        void ProcessBlock(const std::vector<byte>& block);
        void ProcessBlock(const void* block, int size);
        dword Release(); // подсчет CRC закончен, получить результат.
        void Reset();

    };
} // namespace BfBootCore

#endif
