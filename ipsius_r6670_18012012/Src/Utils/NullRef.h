#pragma once

#include "ErrorsSubsystem.h"

namespace Utils
{
    
    template<class T>
    T& NullRef()
    {
        ESS_HALT("Null ref");
        return *((T*)(0));
    }
        
}  // namespace Utils