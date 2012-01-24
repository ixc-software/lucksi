#include "stdafx.h"

#include "Platform/Platform.h"
#include "Utils/DelayInit.h"
#include "StatisticBase.h"

// ------------------------------------------------

namespace
{
    Platform::Mutex *GPMutex;
    Utils::DelayInit MutexInit(GPMutex);

}  // namespace


// ------------------------------------------------

namespace Utils
{

    // template<class TClass>
    //Utils::StatMetaFieldList<TClass> StatisticBase<TClass>::PClassMeta = 0;

    Platform::Mutex& StatisticBaseMetaMutex()
    {
        ESS_ASSERT(GPMutex != 0);
        return *GPMutex;
    }
    
}  // namespace Utils

