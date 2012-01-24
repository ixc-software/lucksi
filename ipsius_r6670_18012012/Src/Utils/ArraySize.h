#pragma once 

namespace Utils
{
    /*
    template<T>
    struct ArraySize
    {
        static const size_t Size = sizeof(T) / sizeof(T[0]); 
    } */
    
    /*        
    template<class T>
    size_t ArraySize(const T &arr)
    {
        return sizeof(T) / sizeof(arr[0]);
    } */
    
    template<typename T, int size> 
    int ArraySize(T (&)[size]) { return size; }; 
    
    #define UTILS_ARRAY_SIZE(arr) ( sizeof(arr) / sizeof(arr[0]) )
        
}


