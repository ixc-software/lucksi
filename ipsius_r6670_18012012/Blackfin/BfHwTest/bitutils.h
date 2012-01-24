#ifndef __BIT_UTILS__
#define __BIT_UTILS__

#include "macros.h"

    //методы работы с битами в байтах памяти        
   	//--------------------------------------------        
    
    void SetBit(byte &addr, byte bit);
    void ClearBit(byte &addr, byte bit);
    bool TestBit(byte val, byte bit);

#endif







