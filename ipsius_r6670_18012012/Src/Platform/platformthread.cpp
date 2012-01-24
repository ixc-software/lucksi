#include "stdafx.h"

#include "ThreadUtils.h"
#include "Utils/ThreadNames.h"

#include "Platform/PlatformThread.h"

// ---------------------------------------------------

namespace Platform
{
    
    void Thread::ThreadBody()
    {
        Utils::ThreadNamesLocker locker(GetCurrentThreadID(), m_name);

        run();  // run user code
    }

    std::string Thread::GetCurrentThreadName()
    {
        return ThreadUtils::GetCurrentThreadName();
    }

    std::string Thread::MakeThreadName( const std::string &name )
    {
        return ThreadUtils::MakeThreadName(name);
    }

    std::string Thread::ThreadIdToStr( ThreadID id )
    {
        return ThreadUtils::ThreadIdToStr(id);
    }

}  // namespace Platform

