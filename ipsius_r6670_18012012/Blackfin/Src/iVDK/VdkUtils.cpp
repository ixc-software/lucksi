#include "VDK.h"
#include "VdkUtils.h"

// -------------------------------------

namespace iVDK
{

	void Sleep(int tickInterval)
	{
		VDK::Sleep(tickInterval);
	}
	
    int GetSystemTickCount()
    {
    	return VDK::GetUptime();
    }

		
}  // namespace iVDK
