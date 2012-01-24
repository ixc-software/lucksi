
#include "stdafx.h"
#include "PlatformPOSIX.h"
#include "execinfo.h"

extern "C" char *cplus_demangle (const char *mangled, int options);

namespace Platform
{

    namespace detail
    {
        int backtrace(void **buffer, int size)
        {
            return ::backtrace(buffer, size);
        }

        char **backtraceSymbols(void *const *buffer, int size)
        {
            return backtrace_symbols(buffer, size);
        }

        char* cplusDemangle(const char *symbol, size_t size)
        {
            return cplus_demangle(symbol, size);
        }

    }

} // namespace Platform
