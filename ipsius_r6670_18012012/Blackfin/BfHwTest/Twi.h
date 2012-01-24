#ifndef __BFTWI__
#define __BFTWI__


#include "macros.h"

namespace BfDev
{	
        
    /*
        BfTwi class
        .....
    */                
               
	enum
	{
        CTransferOK = 0,	    
        CAddrNACK = 1,
        CDataNACK = 2,  
        CTxBuffOVR = 4,                      
        CRxBuffOVR = 8,        
	};
    
    
	class BfTwi
	{		
			
	public:

		BfTwi();
		~BfTwi();		

		
	    bool WaitAcknowledge(word slaveAddress);		
		byte WriteData(word slaveAddress, byte *buff, byte count);
		byte ReadData(word slaveAddress, byte *buff, byte count);        
	    
					 				       		
    private:    

	    byte WaitAvailableToWrite();
	    byte WaitTransferComplete(bool checkAddressNACK = true);
	    byte GetMasterStatus();
	    byte WaitRxData();
	    void WaitBusRelease();
	    word TransferCount(byte count);
	        	
	};

} //namespace BfDev

#endif

