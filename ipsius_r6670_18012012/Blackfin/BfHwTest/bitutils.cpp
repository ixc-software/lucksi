#include "BitUtils.h"

    //������ ������ � ������ � ������ ������        
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







