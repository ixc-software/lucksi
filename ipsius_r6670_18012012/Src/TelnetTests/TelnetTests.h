#ifndef __TELNETTEST__
#define __TELNETTEST__

#include "Utils/QtHelpers.h"
#include "Utils/IBasicInterface.h"

namespace TelnetTests
{
    class ITelnetTestLogger : public Utils::IBasicInterface
    {
    public:
        virtual void Add(const QString &data, bool asLine = false) = 0;
        virtual QString Get() const = 0;
        virtual bool IsSilentMode() const = 0;
    };

    // --------------------------------------------
    
    void Display(const QByteArray &data, const std::string &prevComment = "");

    void TestOk(const std::string &comment = "");
    
    // telnet tests
    void TelnetDataCollectorTest(ITelnetTestLogger &testLog);

    void TelnetClientServerTest(ITelnetTestLogger &testLog); // client-servet system

    void TelnetHelpersTest(ITelnetTestLogger &testLog);

    void TelnetUtilsTest(ITelnetTestLogger &testLog);

    void TelnetOptionListTest(ITelnetTestLogger &testLog);


    // -----------

    void RunTelnetServer(ITelnetTestLogger &testLog);
    void RunTelnetClient();

} // telnetTest

#endif
