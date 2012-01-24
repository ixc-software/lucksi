#include "stdafx.h"

#include "Platform/Platform.h"
#include "Utils/IntToString.h"

#include "MfTrace.h"

// -----------------------------------------------

namespace MiniFlasher
{
    
    void MfTrace::Trace(const void *pData, int size)
    {
        if (!m_enabled) return;

        bool dataCut = false;
        int bytes = size;
        if ((m_bytesLimit > 0) && (bytes > m_bytesLimit)) 
        {
            bytes = m_bytesLimit;
            dataCut = true;
        }
        ESS_ASSERT(bytes > 0);
        ESS_ASSERT(bytes <= size);

        std::cout << m_prefix << "#" << m_counter++ << " "
            << Utils::DumpToHexString(pData, bytes);

        if (dataCut)
        {
            std::cout << " (...) [" << size << "]";
        }

        std::cout << std::endl;
    }
    
}  // namespace MiniFlasher

