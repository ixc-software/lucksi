#include "stdafx.h"
#include "Ds2155hal.h"


namespace Ds2155
{    
	
    enum
    {
        // AccessTime
        CRefTimenS             	   = 1000,
    };

    
    //---------------------------------------------------------------------    		
	
	Ds2155hal::Ds2155hal():
		m_accessTimer(BfTimerCounter(BfTimerCounter::GetFreeTimer()))
	{
		AllDs2155BoardsResetSet();
		m_sleepTicks = m_accessTimer.NsToCounter(CRefTimenS);
		
	}
        	
    //---------------------------------------------------------------------    		
	
	byte Ds2155hal::Read(byte *m_baseAddress, byte addr)
	{
        return (*(m_baseAddress + (addr<<1)));				
	}
		
    //---------------------------------------------------------------------    	
				
	void Ds2155hal::Write(byte *m_baseAddress, byte addr, byte data)
	{

		(*(m_baseAddress + (addr<<1))) = data;
				
	}

   	//--------------------------------------------    
	
	byte Ds2155hal::ReadStatus(byte *m_baseAddress, byte addr, byte mask)
    {
		Write(m_baseAddress,addr, mask);	
		m_accessTimer.Sleep(100);
		return Read(m_baseAddress,addr);	    		    			
    }	    
		
	
	
    //---------------------------------------------------------------------    		
	
	void Ds2155hal::AllDs2155BoardsResetSet()
	{
    	*pPORTFIO_DIR		|= 0x4000;
    	*pPORTFIO_CLEAR		= 0x4000;
    	
    	*pPORTFIO_DIR		|= 0x0010; //for SednaBoard
    	*pPORTFIO_SET 		= 0x0010;
    	
//    	*pPORTFIO_CLEAR		= 0x4000;
//    	*pPORTFIO_SET 		= 0x4000;		
	}
		
    //---------------------------------------------------------------------    			
};




