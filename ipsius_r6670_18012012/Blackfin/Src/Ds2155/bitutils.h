#ifndef __BIT_UTILS__
#define __BIT_UTILS__


namespace Ds2155
{
    using namespace Platform;
        
    //методы работы с битами в байтах памяти        
   	//--------------------------------------------        
    
    static void SetBit(byte &addr, byte bit)
    {
    	addr |= 1<<bit;
    }	
	
   	//--------------------------------------------

    static void ClearBit(byte &addr, byte bit)
    {
    	addr &= ~(1<<bit);
    }

   	//--------------------------------------------

    static bool TestBit(byte val, byte bit)
    {
    	return val & (1<<bit);
    }
    	
};

#endif







