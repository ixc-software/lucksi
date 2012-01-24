#include "stdafx.h"
#include "LogTest.h"
#include "LogImplWString.h"

// -----------------------------------------------------------

namespace
{
    using namespace iLog;
    using namespace iLogWStr;

    void TestFn()
    {
        LogStore logStore;
        LogSession session(logStore, "test");

        {
            LoggerStream logger(session);
            logger.out() << "Test" << 15 << L" X";
        }

    }

}  // namespace

// -----------------------------------------------------------

namespace iLog
{

    void iLogTest()
    {
        TestFn();
    }

}  // namespace iLog


