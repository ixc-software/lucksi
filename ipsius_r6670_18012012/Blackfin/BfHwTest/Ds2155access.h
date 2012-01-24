#ifndef __DS2155_HAL__
#define __DS2155_HAL__

#include "macros.h"

       
	byte DSRead(byte addr);
	void DSWrite(byte addr, byte data);				
//	byte DSReadStatus(byte addr, byte mask);
	void DSReset();
	
	void DSMaskSet(byte addr, byte mask);
    void DSMaskClear(byte addr, byte mask);

#endif
