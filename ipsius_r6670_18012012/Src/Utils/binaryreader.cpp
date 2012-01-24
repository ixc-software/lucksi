#include "stdafx.h"
#include "BinaryReader.h"
#include "MemReaderStream.h"

// --------------------------------------

namespace Utils
{

    IBinaryReader* CreateBinaryReader(const void *pData, int size)
    {
        MemReaderStream *pStream = new MemReaderStream(pData, size);
        return new BinaryReaderEx<MemReaderStream>(pStream);
    }


    IBinaryReader* CreateBinaryReader(const std::vector<Platform::byte> &data)
    {
        ESS_ASSERT(data.size() > 0);
        return CreateBinaryReader(&data[0], data.size());
    }
    
}  // namespace Utils
