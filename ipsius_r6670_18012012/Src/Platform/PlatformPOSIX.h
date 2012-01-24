#pragma once

namespace Platform
{
    namespace detail
    {
        int backtrace(void **buffer, int size);

        char **backtraceSymbols(void *const *buffer, int size);

        char* cplusDemangle(const char *symbol, size_t size);
    }
}
