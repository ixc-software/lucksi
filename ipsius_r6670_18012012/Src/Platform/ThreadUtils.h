#pragma once 

// WARNING! include this file only in PlatformThread.cpp 

#include "Platform/PlatformThread.h"
#include "Utils/ThreadNames.h"

namespace Platform
{

    struct ThreadUtils
    {

        static std::string GetCurrentThreadName()
        {
            return Utils::ThreadNames::Instance().ThreadName( Thread::GetCurrentThreadID() );
        }

        static std::string ThreadIdToStr(Thread::ThreadID id)
        {
            typedef int T;
            BOOST_STATIC_ASSERT( sizeof(T) == sizeof(Thread::ThreadID) );

            std::ostringstream oss;
            oss << (T)id;
            return oss.str();
        }

        static std::string MakeThreadName(const std::string &name)
        {
            const bool CAddParent = true;

            std::string res = name;
            if (CAddParent) res += "/" + GetCurrentThreadName();

            return res;
        }

    };

}  // namespace Platform