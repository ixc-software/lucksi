#include "stdafx.h" 
#include "StatisticElement.h"

// ---------------------------------------------

namespace Utils
{

    namespace StatisticElementDetail
    {

        void FakeInit()
        {
            // nothing, for this file include in project
        }

    }  // namespace StatisticElementDetail


    // Forced template instance
    template class StatisticElement<int, Platform::int64>;  // StatElementForInt
    
}  // namespace Utils

