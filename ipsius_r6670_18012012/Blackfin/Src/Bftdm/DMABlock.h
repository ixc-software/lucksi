#ifndef __TDM_DMA_BLOCK_H__
#define __TDM_DMA_BLOCK_H__

#include "Platform/Platform.h"
//#include <ccblkfn.h>

namespace BfTdm
{
                       
    using namespace Platform;                  
    
    class DMABlock
    {    	    
    public:    	                

        DMABlock(word block_size, DMABlock *prev);
        ~DMABlock();
        
        void SetNext(DMABlock *pnext);
        
        void *GetDescriptor();
        word *GetBufferCh0() const;
        word *GetBufferCh1() const;
        
    private:        

        //DMA дескрипторы для приема и передачи
        struct DMA_descriptor
        {
            DMA_descriptor* m_nextDescriptor;
            word            *m_startAddress;
        };	    	    	            

        DMA_descriptor  m_descriptor;                                
        word            *m_buffer;                                                
        word            *m_bufferCh0;
        word            *m_bufferCh1;
    };		
} //namespace BFTdm

#endif

