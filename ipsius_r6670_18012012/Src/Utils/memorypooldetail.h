#ifndef __MEMORYPOOLDETAIL__
#define __MEMORYPOOLDETAIL__

#include "ProjConfigLocal.h"

namespace Utils
{    
    using ProjConfig::CfgMemoryPool;
	
    namespace Detail
    {
        // template for type selection thru bool between 
        // thread safe/unsafe strategy
        template <bool threadSafe = true> 
        class ThreadStrategyClass 
        {
        public:

            typedef Platform::Mutex       Mutex;
            typedef Platform::MutexLocker Locker;

        };

        template<> 
        class ThreadStrategyClass<false>  
        {
        public:

            class Mutex {};

            // RAII locker for mutex 
            class Locker
            {
            public:
                Locker(Mutex &m) {}
            };


        };

    }  // namespace Detail

    typedef Detail::ThreadStrategyClass<CfgMemoryPool::CEnableThreadSafe> DefaultThreadSafeStrategy;
		
}  // namespace Utils

#endif

