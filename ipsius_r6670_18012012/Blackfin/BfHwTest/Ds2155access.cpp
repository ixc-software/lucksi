#include "Ds2155access.h"
#include "Timer0.h"
	
	enum
    {
		CBaseAddress = 0x20300000UL,
    };        	
	
    enum
    {
        // AccessTime
        CRefTimenS             	   = 1000,
    };

    //---------------------------------------------------------------------    		
	
	byte DSRead(byte addr)
	{
		byte *ptr  = (byte *)CBaseAddress;
        return (*(ptr + (addr<<1)));				
	}
		
    //---------------------------------------------------------------------    	
				
	void DSWrite(byte addr, byte data)
	{

		byte *ptr  = (byte *)CBaseAddress;		
		(*(ptr + (addr<<1))) = data;
				
	}

   	//--------------------------------------------    
/*	
	byte DSReadStatus(byte addr, byte mask)
    {
		Write(m_baseAddress,addr, mask);	
		m_accessTimer.Sleep(100);
		return Read(m_baseAddress,addr);	    		    			
    }	    			
*/	
    //---------------------------------------------------------------------    		
	
	void DSReset()
	{
    	*pPORTFIO_DIR		|= 0x4000;
    	*pPORTFIO_CLEAR		= 0x4000;    	
	}
	
    //---------------------------------------------------------------------    			
	
	void DSMaskSet(byte addr, byte mask)
	{
    	DSWrite(addr, DSRead(addr) | mask);		
	}

   	//--------------------------------------------    
	
    void DSMaskClear(byte addr, byte mask)
    {
    	DSWrite(addr, DSRead(addr) & ~mask);    	
    }
    
   	//--------------------------------------------    		
