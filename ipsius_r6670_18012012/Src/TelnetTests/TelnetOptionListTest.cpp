
#include "stdafx.h"
#include "TelnetTests.h"

#include "Telnet/TelnetCommands.h"
#include "Telnet/TelnetOptionList.h"

#include "Utils/ErrorsSubsystem.h"

namespace 
{
    enum
    { 
        CTestOpt = Telnet::OPT_TTYPE,
    };


} // namespace

namespace TelnetTests
{
    using namespace Telnet;

    void TelnetOptionListTest(ITelnetTestLogger &testLog)
    {
        testLog.Add("TelnetOptionListTest ...", true);
        
        TelnetOptionList optList;

        // test IsExist()
        optList.Add(CTestOpt);
        size_t prevSize = optList.Size();
        TUT_ASSERT(optList.IsExists(CTestOpt));

        // test Add()
        optList.Add(CTestOpt);
        TUT_ASSERT(optList.Size() == prevSize);

        // test Enable()
        optList.Enable(CTestOpt);
        TUT_ASSERT(optList.IsEnabled(CTestOpt));

        // test Disable
        optList.Disable(CTestOpt);
        TUT_ASSERT(!optList.IsEnabled(CTestOpt));

        // test Remove
        optList.Remove(CTestOpt);
        TUT_ASSERT(!optList.IsExists(CTestOpt));

        testLog.Add("OK!", true);
    }

} // namespace TelnetTests

