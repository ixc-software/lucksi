#ifndef DEBUGALLOCATIONCOUNTER_H
#define DEBUGALLOCATIONCOUNTER_H

#ifdef ENABLE_GLOBAL_NEW_DELETE
#include "GlobalNewDelete.h"

namespace Utils
{
    class AllocCounter
    {
    public:
        static int  Get()
        {
            return GetMemAllocatorCounter();
        }

        static bool ModeEnabled()
        {
            return true;
        }
    };
} // namespace Utils


#else

namespace Utils
{
    class AllocCounter
    {
    public:
        static int  Get()
        {
            return 0;
        }

        static bool ModeEnabled()
        {
            return false;
        }
    };
} // namespace Utils

#endif

#endif

