#ifndef __FULLENUMRESOLVER__
#define __FULLENUMRESOLVER__

#include "Platform/Platform.h"
#include "Utils/IntToString.h"

namespace Utils
{

    template<class TEnum>
    std::string EnumResolve(TEnum val, bool *pError = 0)
    {
        if (pError != 0) 
        {
            *pError = true;
        }

        return Utils::IntToString(val);
    }

    
}  // namespace Utils

#endif