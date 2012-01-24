
#include "stdafx.h"
#include "LocalTestSocketRegister.h"

namespace 
{
    TestUtils::LocalSocketRegisterClass *GSocketRegister;

} // namespace 

// ----------------------------------

namespace TestUtils
{
    LocalSocketRegisterClass& LocalSocketRegister()
    {
        if (GSocketRegister == 0) GSocketRegister = new LocalSocketRegisterClass();
        return *GSocketRegister;
    }

} // namesapce TestUtils

