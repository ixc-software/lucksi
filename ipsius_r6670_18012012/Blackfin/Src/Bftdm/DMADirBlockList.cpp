#include "stdafx.h"

#include <ccblkfn.h>
#include "DMADirBlockList.h"
#include "DMABlock.h"
    
namespace BfTdm
{
    using namespace Platform;
            
    //---------------------------------------------------------------------        
            
    DMADirBlockList::DMADirBlockList(volatile unsigned short *m_pDMA_IRQ_Status):
        m_numberOfProcessedBlocks(0),
        m_newBlock(false),
        m_pDMA_IRQ_Status(m_pDMA_IRQ_Status)
    {                       

    }
    
    //-----------------------------------------------------------                
    
    DMADirBlockList::~DMADirBlockList()
    {
        DMABlockList::iterator i = m_blockList.begin();
        
        for (; i != m_blockList.end(); ++i)
            delete (*i);                
    }
    
    //-----------------------------------------------------------            
            
    void *DMADirBlockList::GetFirstDescriptorP() const
    {
        return m_blockList.front()->GetDescriptor();
    }
    
    //-----------------------------------------------------------                

    void DMADirBlockList::ClearNewBlockFlag()
    {
         m_newBlock = false;
    }
        
    //-----------------------------------------------------------            
    
    bool DMADirBlockList::isNewBlockFlag() const
    {
        return m_newBlock;
    }
    
    //-----------------------------------------------------------                
    
    void DMADirBlockList::push_back(DMABlock *block)
    {
        m_blockList.push_back(block);
    }
    
    //-----------------------------------------------------------        
    
    DMABlock *DMADirBlockList::front()
    {
        return m_blockList.front();
    }
    
    //-----------------------------------------------------------    
    
    void DMADirBlockList::DMAIntProcess()
    {        
       	*m_pDMA_IRQ_Status |= 0x0001;
    	ssync();
        
    	m_numberOfProcessedBlocks++; // увеличим счетчик блоков    	
    	m_newBlock = true;
    }
    
    //-----------------------------------------------------------        

    int  DMADirBlockList::GetProcessedBlocksCount() const
    {
        return m_numberOfProcessedBlocks;
    }
    
    //-----------------------------------------------------------            
           
} //namespace BFTdm
