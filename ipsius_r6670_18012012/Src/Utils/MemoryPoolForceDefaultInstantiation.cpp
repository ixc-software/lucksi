#include "stdafx.h"
#include "MemoryPoolBody.h"

namespace Utils
{

    //Forced class template instantiation    
    template class FixedMemBlockPool<Detail::ThreadStrategyClass<false>, false>;
    template class FixedMemBlockPool<Detail::ThreadStrategyClass<false>, true>;
    template class FixedMemBlockPool<Detail::ThreadStrategyClass<true>, false>;
    template class FixedMemBlockPool<Detail::ThreadStrategyClass<true>, true>;

} // namespace Utils
