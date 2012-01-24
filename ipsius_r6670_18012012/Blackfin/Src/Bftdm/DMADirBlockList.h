#ifndef __TDM_DMA_DIR_BLOCK_LIST_H__
#define __TDM_DMA_DIR_BLOCK_LIST_H__

#include "Platform/Platform.h"
#include "DMABlock.h"

namespace BfTdm
{                       
    using namespace Platform;                  
    
    class DMADirBlockList  : boost::noncopyable
    {    	                    
    
    public:    	            
    
        DMADirBlockList(volatile unsigned short *m_pDMA_IRQ_Status);
        ~DMADirBlockList();        
        
        int  GetProcessedBlocksCount() const;                
        void *GetFirstDescriptorP() const; 
        
        void DMAIntProcess();
        
        bool isNewBlockFlag() const;                
        void ClearNewBlockFlag();                        
        
        void push_back(DMABlock *block);
        DMABlock *front();                       
        
    private:        	    
        
       	typedef std::vector<DMABlock*>  DMABlockList;
	    DMABlockList     m_blockList;                
	    
        volatile unsigned short *m_pDMA_IRQ_Status;        
        bool m_newBlock;                   
        int  m_numberOfProcessedBlocks;
    };
		
} //namespace BFTdm

#endif

