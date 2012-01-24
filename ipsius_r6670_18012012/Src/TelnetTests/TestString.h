#ifndef __TELNETTESTSTRING__
#define __TELNETTESTSTRING__

#include "stdafx.h"
#include "Telnet/TelnetHelpers.h"
#include "Telnet/TelnetCommands.h"
#include "Telnet/TelnetUtils.h"
#include "Telnet/TelnetClient.h"

namespace TelnetTests
{
    const int CWinWidth = Telnet::TelnetClient::TelnetInitParams().getWinWidth();
    const int CWinHeight = Telnet::TelnetClient::TelnetInitParams().getWinHeight();

    const QByteArray CTextBegin = "Some_text_begin ";
    const QByteArray CTextMid = "Some_text_mid ";
    const QByteArray CTextEnd = "Some_text_end ";

    const Telnet::Location CLocation = Telnet::Location();
    const QString CTerminalType = "XTERM"; 

    // ---------------------------------------

    QByteArray TestStringConnect1();
    QByteArray TestStringConnect2();
    QByteArray TestString();

    // ----------------------------------------
    // unify data for test TelnetProto logic
    class TestData
    {
    public:
        static QByteArray ServerSocketOutputData(); // == client socket input
        static QString ServerShellOutputData();
        static QByteArray ClientSocketOutputData();
        static QString ClientUserOutputData();
        static QString ClientUserData();
        static QString ServerShellData();
        static QString DataCheckSuccessful();
        static QString CheckClientInterface();
        static QString FinishIndData();
        static QString FinishReqData();
        static QString FinishConfData();
    };

} // namspace TelnetTest 

#endif
