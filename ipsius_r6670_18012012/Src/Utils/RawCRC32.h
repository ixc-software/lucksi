#ifndef __RAWCRC32__
#define __RAWCRC32__

#include "Platform/Platform.h"

namespace Utils
{

    struct RawCRC32
    {
        static const Platform::dword InitialValue = 0xFFFFFFFF;
    };

    Platform::dword UpdateCRC32(const void *pBlock, int length, 
        Platform::dword crc = RawCRC32::InitialValue, bool finalize = true);

    template<class T>
    Platform::dword UpdateCRC32(const std::vector<T> &vec, 
        Platform::dword crc = RawCRC32::InitialValue,
        bool finalize = true)
    {
        int itemSize = sizeof(T); // platform data align - ?
        return UpdateCRC32(&vec[0], vec.size() * itemSize, crc, finalize);
    }

    void TestRawCrc32();
    
}  // namespace Utils

#endif

