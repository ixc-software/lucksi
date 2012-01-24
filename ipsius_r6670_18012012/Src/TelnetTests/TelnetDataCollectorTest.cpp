
#include "stdafx.h"

#include "TelnetTests.h"
#include "Telnet/TelnetCommands.h"
#include "Telnet/ITelnet.h"
#include "Telnet/TelnetDataLogger.h"
#include "Telnet/TelnetDataCollector.h"


#include "Utils/Random.h"
#include "Utils/QtHelpers.h"

// #include "Utils/ErrorsSubsystem.h"

namespace
{
    using namespace Telnet;
    using namespace TelnetTests;
    using namespace Utils;

    // consts 
    enum usedCMDandOPT
    {
        CCmd = CMD_WILL,
        COpt = OPT_NAWS,
        CXopt = XOPT_TRANSMIT_CODEC,

        CRandomVal = 5,

        CWinHeight = 600,
        CWinWidth = 800,
    };

    const QByteArray CTestStr = "Hello, my friend!";
    const QByteArray CTestCmdParams = TypeConvertion::ToQByteArray(CWinWidth) 
                                      + TypeConvertion::ToQByteArray(CWinHeight);
    const QByteArray CTestCmdParamsX = "Windows-1251";

    // class for test TelnetDataCollector
    class TestTelnetProto : public ITelnetDataCollectorEvents,
                            public virtual SafeRefServer
    {
        TelnetDataLogger m_logger;
        TelnetDataCollector m_collector;
        QByteArray m_testStrCollector;
        
    // implement ITelnetDataCollectorEvents
    private:
        void ParseCmd(byte cmd, byte opt)
        {
            TUT_ASSERT(cmd == CCmd);
            TUT_ASSERT((opt == COpt) || (opt == CXopt));
        }

        void ParseSb(byte opt, const QByteArray &params)
        {
            TUT_ASSERT((opt == COpt) || (opt == CXopt));
            TUT_ASSERT((params == CTestCmdParams) 
                       || (params == CTestCmdParamsX));
        }

        void ParseText(const QByteArray &data)
        {
            m_testStrCollector += data;
        }

        TelnetDataLogger& GetLogger()
        {
            return m_logger;
        }
        
    public:
        TestTelnetProto() : 
          m_logger("", true),
          m_collector(this/*, m_logger*/)
        {
        }

        void Run(const QByteArray &data)
        {
            Random random(CRandomVal);
            int i = 0;
            while (i <= data.size())
            {
                int val = random.Next(5);
                m_collector.ProcessSocketData(data.mid(i, val));
                i += val;
            }

            TUT_ASSERT(m_testStrCollector == CTestStr);
        }
    };

} // namespace

// -----------------------------------------

namespace TelnetTests
{
    void TelnetDataCollectorTest(ITelnetTestLogger &testLog)
    {
        testLog.Add("TelnetDataCollectorTest ...", true);
        QByteArray data = TelnetHelpers::MakeCmd(CCmd, COpt);
        data += TelnetHelpers::DoubleIacs(CTestStr);
        data += TelnetHelpers::MakeClientSb(COpt, CTestCmdParams);
        data += TelnetHelpers::MakeXoptCmd(CCmd, CXopt);
        data += TelnetHelpers::MakeXoptSb(CXopt, CTestCmdParamsX);

        TestTelnetProto proto;
        proto.Run(data);

        testLog.Add("OK!", true);
    }

} // namespace TelnetTest
