#include "stdafx.h"

#include "UserBlock.h"
#include "BfDev/SysProperties.h"

namespace
{
    enum
    {
        CEnableRxFlush = true,
    };

    /*
    #define FLUSH(P)  {asm volatile("FLUSH[%0++];":"+p"(P));}
    #define FLUSHINV(P)  {asm volatile("FLUSHINV[%0++];":"+p"(P));}
    #define SIMPLEFLUSHINV(P)  {ssync(); asm volatile("FLUSHINV[%0++];"::"#p"(P)); ssync();}

    
    void FlushArea(void *start, void *nd)
    {
    	start = (void *)(((unsigned int)start)&(~31));
    	ssync();
    	while (start<nd)
    		FLUSH(start);
    	ssync();
    }
        
    void FlushInvArea(void *start, void *nd)
    {
    	start = (void *)(((unsigned int)start)&(~31));
    	ssync();
    	while (start<nd) FLUSHINV(start);
    	ssync();
    }

    void InvalidateBlock(void *pBlock, int blockSize)
    {
        char *p = (char*)pBlock;
        FlushArea(p, p + blockSize);
    } */

}  // namespace


namespace BfTdm
{
    using namespace Platform;

    //---------------------------------------------------------------------            
                
    UserBlock::UserBlock(word tsblockSize,
                         word timeSlotsCount,
                         word *txBuff,
                         word *rxBuff,
                         bool T1Mode,
                         bool byteDataMode):
        m_TXpBuff(txBuff),
        m_RXpBuff(rxBuff),
        m_usedByUser(false),
        m_txBuffModified(false),
        m_TSblockSize(tsblockSize),
        m_timeSlotsCount(timeSlotsCount),
        m_T1Mode(T1Mode),
        m_byteDataMode(byteDataMode),
        m_collision(false)
    {
        
    }    
    
    //---------------------------------------------------------------------        
                
    void UserBlock::SetUserUsing()
    {
        if (m_collision) return;

        m_usedByUser = true;    
        m_txBuffModified = false;

        if (CEnableRxFlush) 
        {
            int size = m_TSblockSize * m_timeSlotsCount * 2;

            // InvalidateBlock(m_RXpBuff, size);
            BfDev::SysProperties::Instance().MemFlushInv(m_RXpBuff, size);
        }
    }
        
    //---------------------------------------------------------------------                
    
    void UserBlock::UserFree()
    {
        m_usedByUser = false;
    }
    
    //---------------------------------------------------------------------                    
    
    word UserBlock::GetTSBlockSize() const
    {
        return m_TSblockSize;
    }
            
    //---------------------------------------------------------------------                    
        
    word *UserBlock::GetTxBuffPointer() const
    {
        return m_TXpBuff;
    }

    //---------------------------------------------------------------------                        
    
    word *UserBlock::GetRxBuffPointer() const
    {
        return m_RXpBuff;
    }
         
    
    //---------------------------------------------------------------------                        

    void UserBlock::CopyRxTSBlockTo(byte timeSlot, byte *tsBuff, word offset, word tsSampleCount)
    {
   		ESS_ASSERT(m_byteDataMode);	                    
        CopyRxTSBlock(timeSlot, tsBuff, offset, tsSampleCount);
    }            
    
    //---------------------------------------------------------------------                        

    void UserBlock::CopyRxTSBlockTo(byte timeSlot, word *tsBuff, word offset, word tsSampleCount)
    {
   		ESS_ASSERT(!m_byteDataMode);	            
        CopyRxTSBlock(timeSlot, tsBuff, offset, tsSampleCount);
    }    
       
    //---------------------------------------------------------------------                        

    template<class T> void UserBlock::CopyRxTSBlock(byte timeSlot, T *tsBuff, word offset, word tsSampleCount)
    {
	    if(m_T1Mode) timeSlot = ChannelCalc(timeSlot);        
        ESS_ASSERT(timeSlot < m_timeSlotsCount);
   		ESS_ASSERT(tsSampleCount <= (m_TSblockSize - offset) && "Out of Block Size space");
   		   		
   		word *tsPtr = m_RXpBuff + timeSlot;
   		
		tsPtr += m_timeSlotsCount * offset;			   		     		
		
   		for(word i=0; i < tsSampleCount; i++)
   		{
			*tsBuff++ = *tsPtr;
			tsPtr += m_timeSlotsCount;
   		}        
    }    

    //---------------------------------------------------------------------                    
    
    void UserBlock::PutTxTSBlockFrom(byte timeSlot, const byte *tsBuff, word offset, word tsSampleCount)
    {
   		ESS_ASSERT(m_byteDataMode);       
        PutTxTSBlock(timeSlot, tsBuff, offset, tsSampleCount);   		
    }            
    
    //---------------------------------------------------------------------                    
    
    void UserBlock::PutTxTSBlockFrom(byte timeSlot, const word *tsBuff, word offset, word tsSampleCount)    
    {
   		ESS_ASSERT(!m_byteDataMode);	            
        PutTxTSBlock(timeSlot, tsBuff, offset, tsSampleCount);
    }
    
    //---------------------------------------------------------------------                    
    
    template<class T> 
    void UserBlock::PutTxTSBlock(byte timeSlot, const T *tsBuff, word offset, word tsSampleCount)
    {
	    if(m_T1Mode) timeSlot = ChannelCalc(timeSlot);        
        ESS_ASSERT(timeSlot < m_timeSlotsCount);
   		ESS_ASSERT(tsSampleCount <= (m_TSblockSize - offset) && "Out of Block Size space");
   		
   		m_txBuffModified = true;
   		
   		word *tsPtr = m_TXpBuff + timeSlot;
   		
		tsPtr += m_timeSlotsCount * offset;			   		
   		
   		for(word i=0; i < tsSampleCount; i++)
   		{
			*tsPtr = *tsBuff++;
			tsPtr += m_timeSlotsCount;
   		}   		
    }      

    //---------------------------------------------------------------------                

    void UserBlock::CopyTxTSBlockTo( byte timeSlot, byte *tsBuff, word buffSize )
    {
        ESS_ASSERT(tsBuff != 0);
        ESS_ASSERT(buffSize == GetTSBlockSize());

        const word *tsPtr = m_TXpBuff + timeSlot;
        tsPtr += m_timeSlotsCount;			   		

        for(word i = 0; i < buffSize; i++)
        {
            *tsBuff++ = *tsPtr; 
            tsPtr += m_timeSlotsCount;
        }   		
    }
    
    //---------------------------------------------------------------------                

    const word *UserBlock::GetRxBlock(word frameNum) const
    {
        ESS_ASSERT(!m_T1Mode);
        ESS_ASSERT(frameNum < m_TSblockSize);        
        return m_RXpBuff + (frameNum * m_timeSlotsCount);
    }
    
    //---------------------------------------------------------------------                    
    
    word *UserBlock::GetTxBlock(word frameNum) const    
    {   		
        ESS_ASSERT(!m_T1Mode);        
        ESS_ASSERT(frameNum < m_TSblockSize);                
   		return m_TXpBuff + (frameNum * m_timeSlotsCount);
    }                
    
    //---------------------------------------------------------------------                    
    
	void UserBlock::CopySyncTxTo(byte timeSlot, byte *tsBuff,  word offset, word tsSampleCount)
	{
   		ESS_ASSERT(m_byteDataMode);	    
        CopySyncTx(timeSlot, tsBuff, offset, tsSampleCount);
	}
	
    //---------------------------------------------------------------------                    
    
	void UserBlock::CopySyncTxTo(byte timeSlot, word *tsBuff,  word offset, word tsSampleCount)	
	{
   		ESS_ASSERT(!m_byteDataMode);	    	    
        CopySyncTx(timeSlot, tsBuff, offset, tsSampleCount);	    
	}	   
    
    //---------------------------------------------------------------------                    
    
	template<class T> void UserBlock::CopySyncTx(byte timeSlot, T *tsBuff,  word offset, word tsSampleCount)
	{
	    
	    if(m_T1Mode) timeSlot = ChannelCalc(timeSlot);
        ESS_ASSERT(timeSlot < m_timeSlotsCount);
   		ESS_ASSERT(tsSampleCount <= (m_TSblockSize - offset) && "Out of Block Size space");
   		ESS_ASSERT(!m_txBuffModified && "Tx Buffer was modified");   		
   		
   		word *tsPtr = m_TXpBuff + timeSlot;
   		
		tsPtr += m_timeSlotsCount * offset;			   		     		
		
   		for(word i=0; i < tsSampleCount; i++)
   		{
			*tsBuff++ = *tsPtr;
			tsPtr += m_timeSlotsCount;
   		}   		
		
	}
    	
    //-------------------------------------------------------------------    	
	
    byte UserBlock::ChannelCalc(byte t1ts)
    {
        ESS_ASSERT(t1ts < 24);        
        return t1ts + (t1ts/3) + 1;        
    }	

                   
} //namespace BFTdm
