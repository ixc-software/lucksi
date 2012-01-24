#include <ccblkfn.h>
#include <cdefBF537.h>
#include <stdio.h>
#include "UartSimple.h"

#include "Twi.h"

namespace
{
	BfDev::BfTwi *TWIObj = 0;
}


namespace BfDev
{
    	
    //-------------------------------------------------------------------

    BfTwi::BfTwi()
    {
//		ESS_ASSERT(!TWIObj && "TWI Port already created");    	
        if (TWIObj) Send0Msg("TWI already created\n");

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
    
	byte BfTwi::WriteData(word slaveAddress, byte *data, byte count)
	{	    	        	    
	    if (count == 0) return 0;
	    byte txCount=0;
	    *pTWI_MASTER_ADDR = slaveAddress;		
                  	      	            
        byte status = WaitAvailableToWrite();
        if (status) return status;
        *pTWI_XMT_DATA8 = *data++;	    
        txCount++;
        *pTWI_MASTER_CTL =  FAST | MEN | TransferCount(count);        

       
	    for (int i = 0; i<count-1; i++)
	    {
            status = WaitAvailableToWrite();
            if (status) return 0;            
            *pTWI_XMT_DATA8 = *data++;	    	        
            txCount++;            
	    }	    
	    status = WaitTransferComplete();
	    if (status == CTransferOK) return txCount;
        return 0;        
	}
		
    //-------------------------------------------------------------------	
    
	byte BfTwi::ReadData(word slaveAddress, byte *data, byte count)
	{	    	    
	    *pTWI_MASTER_ADDR = slaveAddress;			    
        byte rxCount = 0;

		*pTWI_MASTER_CTL =  FAST | MDIR | MEN | TransferCount(count);

		for(int i = 0; i < count; i++)
		{
		    if(WaitRxData() != CTransferOK) return 0;
            *data++ = *pTWI_RCV_DATA8;
            rxCount++;
		}								
		
        if(WaitTransferComplete() != CTransferOK) return 0;                        		
		
		return rxCount;
	}		
	
    //-------------------------------------------------------------------    		
	
	bool BfTwi::WaitAcknowledge(word slaveAddress)
	{
	    *pTWI_MASTER_ADDR = slaveAddress;			    
	    
        if(WaitAvailableToWrite() != CTransferOK) return false;
        *pTWI_MASTER_CTL =  FAST | MEN;        
        if(WaitTransferComplete(false) != CTransferOK) return false;        
        while(*pTWI_MASTER_STAT & 0x04)
        {
            *pTWI_MASTER_STAT = 0x04;
            *pTWI_MASTER_CTL =  FAST | MEN;
            if(WaitTransferComplete(false) != CTransferOK) return false;
        }
        return true;
	}
	
    //-------------------------------------------------------------------    	
	
	byte BfTwi::WaitAvailableToWrite()	
	{	
        while(*pTWI_FIFO_STAT & 0x02)
        {
	        word status = GetMasterStatus();
	        if (status) return status;
        }
        return CTransferOK;
	}
	
    //-------------------------------------------------------------------    	
	
	void BfTwi::WaitBusRelease()	
	{	
	    while(*pTWI_MASTER_STAT & BUSBUSY);
	}	

    //-------------------------------------------------------------------    	
	
	byte BfTwi::WaitTransferComplete(bool checkAddressNACK)	
	{	
//	    while(!(*pTWI_INT_STAT & MCOMP));
//  	    *pTWI_INT_STAT = MCOMP;  	    
//	    while(*pTWI_MASTER_STAT & BUSBUSY);  	    

	    while(!(*pTWI_INT_STAT & MCOMP))
	    {
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
	    while(!(*pTWI_FIFO_STAT  & 0x04))
	    {
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
