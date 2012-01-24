#ifndef __PLATFORMATOMIC__
#define __PLATFORMATOMIC__

#include "iVDK/CriticalRegion.h"

namespace Platform
{
	
	struct Atomic
    {
	    // change *pw with value dv, returns previous *pw value	
		static int FetchAndAdd(volatile int *pw, int dv)
		{
			iVDK::CriticalRegion::Enter();  // optimization, don't use directly CriticalRegion instance
				
			int val = *pw;
			*pw += dv;
			
			iVDK::CriticalRegion::Leave();		
				
			return val;					
		}
			
		// inc *pw
	    static int Increment(volatile int *pw)
	    {
			iVDK::CriticalRegion::Enter();  // optimization, don't use directly CriticalRegion instance
				
	        ++(*pw);
	        int val = *pw ? !0 : 0;			
	        	
			iVDK::CriticalRegion::Leave();		
				
			return val;
	    }
    	
    };

			
} // namespace Platform

#endif

