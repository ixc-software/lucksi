#pragma once

#include "Utils/IBasicInterface.h"

#include "Platform/PlatformThread.h"

namespace Platform
{
    
    class IProcSnapshot : public Utils::IBasicInterface
    {
    public:
        virtual std::string ThreadPriority(Thread::ThreadID id) = 0;
        virtual int ThreadCount() = 0;
        virtual Thread::ThreadID ThreadAt(int index) = 0;
    };
    
       
}  // namespace Platform