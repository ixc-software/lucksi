
#ifndef __DRISCRIPTTESTS__
#define __DRISCRIPTTESTS__

// DRIScriptTests.h

#include "DriTestsParams.h"

namespace DriTests
{
    void TestIncludeAndDefineExceptions(DriTestsParams params = DriTestsParams());
    void TestCmdLineSpecialParams(DriTestsParams params = DriTestsParams());
    void TestCS_Commands(DriTestsParams params = DriTestsParams());
    void TestObjCommands(DriTestsParams params = DriTestsParams());
    void TestMetaCommands(DriTestsParams params = DriTestsParams());
    void TestIncludeAndDefine(DriTestsParams params = DriTestsParams());
    void TestTest1(DriTestsParams params = DriTestsParams());

    void TestSec(DriTestsParams params = DriTestsParams());
    void TestSecProfile(DriTestsParams params = DriTestsParams());
    
    void DriScriptTests(const DriTestsParams &params = DriTestsParams());
    
} // namespace DriTests

#endif
