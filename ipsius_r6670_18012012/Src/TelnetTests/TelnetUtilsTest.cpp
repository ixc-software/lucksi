
#include "stdafx.h"

#include "TelnetTests.h"

#include "Telnet/TelnetUtils.h"

#include "Telnet/TelnetHelpers.h"
#include "Telnet/TelnetCommands.h"

#include "Utils/QtHelpers.h"

namespace 
{
    typedef Telnet::TypeConvertion T;
    using namespace Telnet;

    // test consts
    const QString CTestStr = "Test string begin !@#$%^&*()_+ 1 end.";
    const QString CTestStrC = "Тестовая строка.";
    const QByteArray CTestArray = TelnetHelpers::MakeXoptCmd(CMD_WILL, 
                                                             XOPT_TRANSMIT_CODEC);

    enum
    {
        CNum = 500,
        CNumHi = CNum >> 8,
        CNumLo = CNum & 0xFF, 
    };

} // namespace

// -------------------------------------

namespace TelnetTests
{
    
    void TelnetUtilsTest(ITelnetTestLogger &testLog)
    {
        typedef Telnet::TypeConvertion T;

        testLog.Add("TelnetUtilsTest ...", true);
        
        // test concat bytes into word
        TUT_ASSERT(T::ToWord(CNumHi, CNumLo) == CNum);

        // for word
        QByteArray num = T::ToQByteArray(CNum);
        TUT_ASSERT((byte)num.at(0) == CNumHi);
        TUT_ASSERT((byte)num.at(1) == CNumLo);

        // AsciiConvertionTest
        // check convertion without codec
        TUT_ASSERT(T::ToAscii(T::FromAscii(CTestStr)) == CTestStr);
        // check convertion with codec
        std::string codec = T::DefaultCodecName();
        TUT_ASSERT(!codec.empty());
        TUT_ASSERT(T::ToAscii(T::FromAscii(CTestStr, codec), codec) == CTestStr);

        /*
        //(?)
        // check convertion with codec
        Display(T::FromAscii(CTestStrC, codec)); // display '?' codes
        TUT_ASSERT(T::ToAscii(T::FromAscii(CTestStrC, codec), codec) == CTestStrC);
        
        std::string codec2 = "KOI8-R";
        TUT_ASSERT(T::FromAscii(CTestStr, codec) != T::FromAscii(CTestStr, codec2));
        //TUT_ASSERT(T::ToAscii(T::FromAscii(CTestStr), codec));
        */

        // UnicodeConvertionTest
        TUT_ASSERT(T::ToUnicode(T::FromUnicode(CTestStr)) == CTestStr);
        
        // BinaryConvertionTest
        TUT_ASSERT(T::FromBinary(T::ToBinary(CTestArray)) == CTestArray);

        testLog.Add("OK!", true);
    }

} // namespace TelnetTest
