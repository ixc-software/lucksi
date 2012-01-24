#include "stdafx.h"
#include "ThreadRunner.h"

namespace iCore
{
    
    void DoFinalizeThreads()
    {
		// manual destroy, 'couse CpuUsage block Idle thread which must clean destroyed thread
        VDK::FreeDestroyedThreads();        
    }

    void ThreadRunner::FinalizeThreads()
    {
        DoFinalizeThreads();
    }

    
}  // namespace iCore
