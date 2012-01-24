#include "BitUtils.h"

    //методы работы с битами в байтах памяти        
   	//--------------------------------------------        
    
    void SetBit(byte &addr, byte bit)
    {
    	addr |= 1<<bit;
    }	
	
   	//--------------------------------------------

    void ClearBit(byte &addr, byte bit)
    {
    	addr &= ~(1<<bit);
    }

   	//--------------------------------------------

    bool TestBit(byte val, byte bit)
    {
    	return val & (1<<bit);
    }    	







