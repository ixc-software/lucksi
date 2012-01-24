#ifndef __BFTWI__
#define __BFTWI__

#include "Platform/Platform.h"
#include "BfDev/BfTimerCounter.h"
#include "Utils/TimerTicks.h"


namespace BfDev
{	    
        
    using namespace Platform;
    /*
        BfTwi class
        .....
    */                                       
    
	class BfTwi : boost::noncopyable
	{		
			
	public:
	
        ESS_TYPEDEF(TransferTimeout);	

		BfTwi();
		~BfTwi();		
		
	    bool WaitAcknowledge(word slaveAddress, word timeoutmS);
		word WriteData(word slaveAddress, void *buff, word count);
		word ReadData(word slaveAddress, void *buff, word count);        	    
					 				       		
    private:    

	    byte WaitAvailableToWrite();
	    byte WaitTransferComplete(bool checkAddressNACK = true);
	    byte GetMasterStatus();
	    byte WaitRxData();
	    void WaitBusRelease();
	    word TransferCount(byte count);
	    
    private:    

		Utils::TimerTicks	m_timer;        	    
	    
	        	
	};

} //namespace BfDev

#endif

