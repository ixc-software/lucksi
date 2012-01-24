#ifndef _BF_ADI_DMA_H_
#define _BF_ADI_DMA_H_

#include "stdafx.h"
#include <services/services.h>
#include "Platform/PlatformTypes.h"

namespace BfDev
{
    class AdiDma : boost::noncopyable
    {
    public:
        AdiDma(int countChannel, void *pCriticalRegionArg = 0);

        ADI_DMA_MANAGER_HANDLE Handle()
        {
            return m_handle;
        }
    private:
        u32 m_countChannel;
        std::vector<Platform::byte> m_memory;
        ADI_DMA_MANAGER_HANDLE m_handle;

    };
};

#endif
