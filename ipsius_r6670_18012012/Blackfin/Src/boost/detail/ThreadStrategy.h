
#ifndef __THREADSTRATEGY__
#define __THREADSTRATEGY__

#include "Platform/Platform.h"

/*
    All thread's strategy classes should have two public static methods:
        static void Increment(int *pWhat);
        static int FetchAndAdd(int *pWhat, int val);
*/

namespace boost
{
    namespace detail
    {
                
        // Thread-safe operations
        template <bool threadSafe = true> 
        class ThreadStrategyClass 
        {
        public:
            static void Increment(int *pWhat)
            {
                Platform::Atomic::Increment(pWhat);
            }
            
            static int FetchAndAdd(int *pWhat, int val)
            {
                return Platform::Atomic::FetchAndAdd(pWhat, val);
            }
        };

        // -------------------------------------

        // Thread-unsafe operations
        template<> 
        class ThreadStrategyClass<false>  
        {
        public:
            static void Increment(int *pWhat)
            {
                ++(*pWhat);
            }
            
            static int FetchAndAdd(int *pWhat, int val)
            {
                int res = *pWhat;
                *pWhat += val;
                
                return res;
            }
        };

        // -------------------------------------

        typedef ThreadStrategyClass<ProjConfig::CfgSharedPtr::CEnableThreadSafety> ThreadStrategy;
        
    } // namesapce detail
    
} // namespace boost


#endif
