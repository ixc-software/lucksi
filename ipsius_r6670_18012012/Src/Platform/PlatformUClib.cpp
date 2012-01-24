
#include "stdafx.h"
#include "PlatformPOSIX.h"

namespace Platform
{
    namespace detail
    {
        int backtrace(void **buffer, int size)
        {
            return 0;
        }

        char **backtraceSymbols(void *const *buffer, int size)
        {
            return 0;
        }

        char* cplusDemangle(const char *symbol, size_t size)
        {
            return 0;
        }

    }
} // namespace Platform
