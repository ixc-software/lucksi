#include "VDK.h"
#include "stdafx.h"  // STL must be after VDK.h :-/
#include "CriticalRegion.h"
#include "iVDK/VdkCheckInit.h"

namespace iVDK
{
	
    CriticalRegion::CriticalRegion()
    {
		VdkCheckInit();
    	Enter();
    }
    
    CriticalRegion::~CriticalRegion()
    {
    	Leave();
    }
    
    void CriticalRegion::Enter()
    {
        VDK::PushCriticalRegion();    	
    }
     
    void CriticalRegion::Leave()
    {
        VDK::PopCriticalRegion();    	
    }

	
	
	
}  // namespace iVDK

