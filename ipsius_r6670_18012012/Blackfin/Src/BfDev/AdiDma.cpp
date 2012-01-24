#include "stdafx.h"
#include "AdiDma.h"

namespace BfDev
{
    AdiDma::AdiDma(int countChannel, void *pCriticalRegionArg) :
        m_countChannel(0),
        m_memory(ADI_DMA_BASE_MEMORY + (ADI_DMA_CHANNEL_MEMORY * (2 + countChannel)))
    {
        ADI_DMA_RESULT result = adi_dma_Init(
            &m_memory[0], 
            m_memory.size(),
            &m_countChannel,
            &m_handle,
            pCriticalRegionArg); 
            
        ESS_ASSERT(result == ADI_DMA_RESULT_SUCCESS );
        ESS_ASSERT(m_countChannel == 2 + countChannel);
    }
};


