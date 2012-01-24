
#include "stdafx.h"
#include "TelnetTests.h"
#include "Telnet/TelnetHelpers.h"
#include "Telnet/TelnetCommands.h"

namespace 
{
    using namespace Telnet;

    enum
    {
        CCmdSize = 3,           // bytes
        CCmdGaSize = 2,
        CServerSbSize = 6,
    };

} // namespace

// -----------------------------------------

namespace TelnetTests
{
    using namespace Telnet;

    void TelnetHelpersTest(ITelnetTestLogger &testLog)
    {
        typedef TelnetHelpers T;

        testLog.Add("TelnetHelpersTest ...", true);
        
        // telnet features tests
        // test DoubleIacs()
        QByteArray text;
        TelnetHelpers::Append(text, CMD_IAC);
        TelnetHelpers::Append(text, CMD_IAC);
        int prevSize = text.size();
        text = T::DoubleIacs(text);
        TUT_ASSERT(text.size() == (prevSize * 2));

        // test CutDoubleIacs()
        TUT_ASSERT(T::CutDoubleIacs(text).size() == prevSize);

        // test command constructors
        TUT_ASSERT(T::MakeCmd(CMD_WILL, OPT_TTYPE).size() == CCmdSize);

        TUT_ASSERT(T::MakeCmd(CMD_GA).size() == CCmdGaSize);

        TUT_ASSERT(T::MakeServerSb(OPT_TTYPE).size() == CServerSbSize); 

        testLog.Add("OK!", true);
    }

} // namespace TelnetTest

