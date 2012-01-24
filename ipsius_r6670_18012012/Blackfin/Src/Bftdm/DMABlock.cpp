#include "stdafx.h"

#include "DMABlock.h"
    
namespace BfTdm
{
    using namespace Platform;
            
    //---------------------------------------------------------------------    
                    
    DMABlock::DMABlock(word block_size, DMABlock *prev):
        m_buffer(new word [block_size]),
        m_bufferCh0(m_buffer),
        m_bufferCh1(m_buffer + block_size/2)
    {    
        m_descriptor.m_startAddress = m_bufferCh0;
        if (prev) prev->SetNext(this);        
    }    
    
    //---------------------------------------------------------------------    
    
    DMABlock::~DMABlock()
    {                
        delete[] m_buffer;
    }
    
    //---------------------------------------------------------------------    
        
    void *DMABlock::GetDescriptor()
    {
        return &m_descriptor;
    }
    
    //---------------------------------------------------------------------        

    word *DMABlock::GetBufferCh0() const
    {
        return m_bufferCh0;
    }
    
    //---------------------------------------------------------------------        

    word *DMABlock::GetBufferCh1() const
    {
        return m_bufferCh1;
    }        
    
    //---------------------------------------------------------------------        
    
    void DMABlock::SetNext(DMABlock *next)
    {
        m_descriptor.m_nextDescriptor = &(next->m_descriptor);        
    }
    
    //-------------------------------------------------------------	                
           
} //namespace BFTdm
