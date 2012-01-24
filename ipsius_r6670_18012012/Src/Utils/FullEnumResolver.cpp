#include "stdafx.h"

#include "Utils/DelayInit.h"

#include "FullEnumResolver.h"

namespace
{

    Utils::FullEnumResolver *GPEnum;
    Utils::DelayInit EnumInit(GPEnum);

}  // namespace

// ------------------------------------------------------

namespace Utils
{
       
    const FullEnumResolver& FullEnumResolver::Instance()
    {
        ESS_ASSERT(GPEnum != 0);
        return *GPEnum;
    }

}  // namespace Utils

