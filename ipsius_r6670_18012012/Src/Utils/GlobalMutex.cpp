#include "stdafx.h"

#include "Platform/PlatformMutex.h"
#include "Platform/PlatformThread.h"

#include "GlobalMutex.h"
#include "DelayInit.h"

// -------------------------------------------------

namespace
{
    typedef Platform::Thread::ThreadID ThreadId;

    enum
    {
        CBadThreadID = 0,
    };

    Platform::Mutex   *PGMutex;
    Utils::DelayInit  MutexInit(PGMutex);
    ThreadId LockedThreadID = (ThreadId)CBadThreadID;
}

// -------------------------------------------------

namespace Utils
{
    
    bool GlobalMutextLock()
    {
        ThreadId id = Platform::Thread::GetCurrentThreadID();

        // deadlock workaround
        // can be dangerous - ?        
        if (LockedThreadID == id)
        {
            // already locked in this thread
            return false;
        }

        ESS_ASSERT(PGMutex != 0);
        PGMutex->Lock();
        LockedThreadID = id;
        return true;
    }

    void GlobalMutextUnlock()
    {
        ESS_ASSERT(PGMutex != 0);
        LockedThreadID = (ThreadId)CBadThreadID;
        PGMutex->Unlock();
    }

    
}  // namespace Utils

