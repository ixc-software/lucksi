
#include "stdafx.h"
#include "LocalSocketRegister.h"

namespace 
{
    TelnetTests::LocalSocketRegisterClass *GSocketRegister;

} // namespace 

// ----------------------------------

namespace TelnetTests
{
    LocalSocketRegisterClass& LocalSocketRegister()
    {
        if (GSocketRegister == 0) GSocketRegister = new LocalSocketRegisterClass();
        return *GSocketRegister;
    }

} // namesapce TelnetTests

