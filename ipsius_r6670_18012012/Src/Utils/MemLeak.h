#pragma once

namespace Utils
{
    
    struct MemLeak
    {
        static void LeakOnce(int size = 64 * 1024)
        {
            char *p = new char[size];
        }    
        
        static void LeakForever()
        {
            while (true) LeakOnce();
        }
    };
    
    
}  // namespace Utils