#include "stdafx.h"

#include "PlatformUtils.h"

namespace Platform
{
    
    dword RevertBytesInDword(dword n)
    {
        return ((n & 0xff) << 24) |
            ((n & 0xff00) << 8) |
            ((n & 0xff0000UL) >> 8) |
            ((n & 0xff000000UL) >> 24);        
    }
    
    word  RevertBytesInWord(word n)
    {
        return ((n & 0xff) << 8) | ((n & 0xff00) >> 8);        
    }
            
}  // namespace Platform

