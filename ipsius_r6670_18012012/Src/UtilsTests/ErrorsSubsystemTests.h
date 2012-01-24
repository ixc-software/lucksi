#ifndef __ERRORSSUBSYSTEMTESTS__
#define __ERRORSSUBSYSTEMTESTS__

#include "Utils/ErrorsSubsystem.h"
#include "TestFw/TestFw.h"

namespace UtilsTests
{
    // single tests
    void TestTutException();        // This test must be first in group
    void TestSimpleException();
    void TestExceptionHook();
    void TestTutHook();
    void TestAbortHook();


    namespace ESSTests
    {
        // register group
        void RegisterTestGroup(TestFw::ILauncher &launcher);

    } // namespace ESSTests

}  // namespace UtilsTests

#endif

