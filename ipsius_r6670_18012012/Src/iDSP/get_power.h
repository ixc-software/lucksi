#ifndef __GET_POWER__
#define __GET_POWER__

#include "Platform/PlatformTypes.h"

namespace iDSP
{
    using Platform::byte;
    using Platform::word;    	
    using Platform::dword;    	
	
    bool GP_ProcessBuff(const byte *pBuff, int count, bool is_A,    // in params
                         int *pFreq, float *pPower);          // out params
                         
    bool GP_Test();
		
}  // namespace iMedia
                            
   
#endif

