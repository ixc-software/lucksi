#include <ccblkfn.h>
#include <cdefBF537.h>
#include <stdio.h>

#include "BfDev/BfTwi.h"

namespace
{
	BfDev::BfTwi *TWIObj = 0;
	
	enum
	{
        CTransferOK = 0,	    
        CAddrNACK = 1,
        CDataNACK = 2,  
        CTxBuffOVR = 4,                      
        CRxBuffOVR = 8,        
	};
	
	enum
	{
	    CTransferTimeOut = 100,
        CWaitRXTimeOut   = 100,  
	};
	
}


namespace BfDev
{
    	
    //-------------------------------------------------------------------

    BfTwi::BfTwi()
    {
		ESS_ASSERT(!TWIObj && "TWI Object already created");    	
		TWIObj = this;
				
		*pTWI_CONTROL = TWI_ENA | 0x0C;
		*pTWI_CLKDIV = 0x0811;
		*pTWI_MASTER_ADDR = 0;				 
    }
         
    //-------------------------------------------------------------------
    
    BfTwi::~BfTwi()
    {
		TWIObj = 0;    	    	
    }
    
    //-------------------------------------------------------------------    
    
	word BfTwi::WriteData(word slaveAddress, void *data, word count)
	{	    	        	    
	    if (count == 0) return 0;
	    byte txCount=0;
	    byte *buff = (byte*)data;
	    *pTWI_MASTER_ADDR = slaveAddress;		
                  	      	            
        if (WaitAvailableToWrite()!=CTransferOK) return 0;
        *pTWI_XMT_DATA8 = *buff++;	    
        txCount++;
        *pTWI_MASTER_CTL =  FAST | MEN | TransferCount(count);        
       
	    for (int i = 0; i<count-1; i++)
	    {
            if (WaitAvailableToWrite() != CTransferOK) return 0;            
            *pTWI_XMT_DATA8 = *buff++;	    	        
            txCount++;            
	    }	    
	    if (WaitTransferComplete() == CTransferOK) return txCount;
        return 0;        
	}
		
    //-------------------------------------------------------------------	
    
	word BfTwi::ReadData(word slaveAddress, void *data, word count)
	{	    	    
	    *pTWI_MASTER_ADDR = slaveAddress;			    
        byte rxCount = 0;
	    byte *buff = (byte*)data;
	    
		*pTWI_MASTER_CTL =  FAST | MDIR | MEN | TransferCount(count);

		for(int i = 0; i < count; i++)
		{
		    if(WaitRxData() != CTransferOK) return 0;
            *buff++ = *pTWI_RCV_DATA8;
            rxCount++;
		}								
		
        if(WaitTransferComplete() == CTransferOK) return rxCount;		
		return 0;
	}		
	
    //-------------------------------------------------------------------    		
	
	bool BfTwi::WaitAcknowledge(word slaveAddress, word timeoutmS)
	{
	    *pTWI_MASTER_ADDR = slaveAddress;			    
	    
        if(WaitAvailableToWrite() != CTransferOK) return false;
        *pTWI_MASTER_CTL =  FAST | MEN;        
        if(WaitTransferComplete(false) != CTransferOK) return false;        
		m_timer.Set(timeoutmS, false);
        while(*pTWI_MASTER_STAT & 0x04)
        {
            if (m_timer.TimeOut()) ESS_THROW(TransferTimeout);
            *pTWI_MASTER_STAT = 0x04;
            *pTWI_MASTER_CTL =  FAST | MEN;
            if(WaitTransferComplete(false) != CTransferOK) return false;
        }
        return true;
	}
	
    //-------------------------------------------------------------------    	
	
	byte BfTwi::WaitAvailableToWrite()	
	{	
		m_timer.Set(CTransferTimeOut, false);
        while(*pTWI_FIFO_STAT & 0x02)
        {
            if (m_timer.TimeOut()) ESS_THROW(TransferTimeout);
	        word status = GetMasterStatus();
	        if (status) return status;
        }
        return CTransferOK;
	}
	
    //-------------------------------------------------------------------    	
	
	void BfTwi::WaitBusRelease()	
	{	
		m_timer.Set(CTransferTimeOut, false);
	    while(*pTWI_MASTER_STAT & BUSBUSY)
	    {
            if (m_timer.TimeOut()) ESS_THROW(TransferTimeout);
	    }
	    
	}	

    //-------------------------------------------------------------------    	
	
	byte BfTwi::WaitTransferComplete(bool checkAddressNACK)	
	{	
		m_timer.Set(CTransferTimeOut, false);
	    while(!(*pTWI_INT_STAT & MCOMP))
	    {
            if (m_timer.TimeOut()) ESS_THROW(TransferTimeout);
	        if(checkAddressNACK)
	        {
    	        word status = GetMasterStatus();
    	        if (status) return status;	            
	        }
	    }
  	    *pTWI_INT_STAT = MCOMP;  	    
        WaitBusRelease();
        return CTransferOK;

	}

    //-------------------------------------------------------------------    	
	
	byte BfTwi::WaitRxData()	
	{	
		m_timer.Set(CWaitRXTimeOut, false);
	    while(!(*pTWI_FIFO_STAT  & 0x04))
	    {
            if (m_timer.TimeOut()) ESS_THROW(TransferTimeout);
	        word status = GetMasterStatus();
	        if (status) return status;
	    }
	    return CTransferOK;	    
	}	

    //-------------------------------------------------------------------    	
	
	byte BfTwi::GetMasterStatus()	
	{	
	    return (*pTWI_MASTER_STAT & 0x3c) >>2;	        	        
	}			
			
    //-------------------------------------------------------------------		
	
	word BfTwi::TransferCount(byte count)
	{
	    word result = count;
	    result <<= 6;
	    return result;
	}
	
	            
} //namespace BfDev
