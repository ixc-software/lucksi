
#include "stdafx.h"

#include "TestString.h"

using namespace Platform;

namespace
{
    QByteArray ToQByteArray(const std::vector<byte> &data)
    {
        QByteArray res;
        //res.reserve(data.size());
        for (size_t i = 0; i < data.size(); ++i)
        {
            res += static_cast<char>(data.at(i));
        }
        return res;
    }
}

// ----------------------------------------

namespace TelnetTests
{
    using namespace Telnet;
    using namespace std;

    QByteArray TestStringConnect1()
    {
        byte array[] = {
            0xFF, 0xFB, 0x1F,
            0xFF, 0xFB, 0x20, 
            0xFF, 0xFB, 0x18, 
            0xFF, 0xFB, 0x27, 
            0xFF, 0xFD, 0x01, 
            0xFF, 0xFB, 0x03, 
            0xFF, 0xFD, 0x03, 
            0xFF, 0xFC, 0x23, 
            0xFF, 0xFC, 0x24, 
            0xFF, 0xFA, 0x1F, 0x00, 0x50, 0x00, 0x18, 0xFF, 0xF0, 
            0xFF, 0xFA, 0x20, 0x00, 0x33, 0x38, 0x34, 0x30, 0x30, 0x2C, 
                                    0x33, 0x38, 0x34, 0x30, 0x30, 0xFF, 0xF0, 
            0xFF, 0xFA, 0x27, 0x00, 0xFF, 0xF0, 
            0xFF, 0xFA, 0x18, 0x00, 0x58, 0x54, 0x45, 0x52, 0x4D, 0xFF, 0xF0
        };

        vector<byte> data(array, (array + sizeof(array) / sizeof(byte)));

        return ToQByteArray(data);
    }

    // ------------------------------------------

    QByteArray TestStringConnect2()
    {
        byte array[] = {
            0xFF, 0xFC, 0x01, 
            0xFF, 0xFC, 0x22, 
            0xFF, 0xFE, 0x05, 
            0xFF, 0xFC, 0x21
        };

        vector<byte> data(array, (array + sizeof(array) / sizeof(byte)));

        return ToQByteArray(data);
    }

    // -------------------------------------------

    QByteArray TestString()
    {
        QByteArray res;
        QByteArray text("Next Request...");

        byte array1[] = {
            0xFF, 0xFC, 0x01, /* IAC WONT ECHO */
            0xFF, 0xFC, 0x22 /* IAC WONT LINEMODE */
        };

        vector<byte> data1(array1, (array1 + sizeof(array1) / sizeof(byte)));

        byte array2[] = {
            0xFF, 0xFC, 0x22, /* IAC WONT LINEMODE */
            0xFF, 0xFC, 0x01 /* IAC WONT ECHO */
        };

        vector<byte> data2(array2, (array2 + sizeof(array2) / sizeof(byte)));

        res += ToQByteArray(data1);
        res += text;
        res += ToQByteArray(data2);

        return res;
    }

    // ----------------------------------------
    // data for test TelnetProto logic
    // those functions to test CaseReplyCommand() in TelnetProtoX


    QByteArray TestData::ServerSocketOutputData() // == client socket input
    {
        typedef TelnetHelpers T;

        QByteArray res = /*T::MakeCmd(CMD_DO, OPT_TRANSMIT_BINARY)*/
                        T::MakeCmd(CMD_DO, OPT_NAWS)
                        + T::MakeServerSb(OPT_NAWS)
                        + T::MakeCmd(CMD_DONT, OPT_TSPEED)
                        + T::MakeCmd(CMD_GA)
                        + T::MakeCmd(CMD_WILL, OPT_SGA)
                        + T::MakeCmd(CMD_DO, OPT_XDISPLOC)
                        + T::MakeServerSb(OPT_XDISPLOC)
                        + T::MakeCmd(CMD_DO, OPT_TTYPE)
                        + T::MakeServerSb(OPT_TTYPE);

        return res;
    }

    // ----------------------------------------

    QString TestData::ServerShellOutputData()
    {
        return QString(CTextBegin + CTextMid + CTextEnd);
    }

    // ----------------------------------------

    QByteArray TestData::ClientSocketOutputData()
    {
        typedef TelnetHelpers T;

        QByteArray nawsParam = TypeConvertion::ToQByteArray(CWinWidth) 
                               + TypeConvertion::ToQByteArray(CWinHeight);

        QByteArray res = /*T::MakeClientSb(OPT_NAWS, nawsParam)
                        + */T::MakeCmd(CMD_WILL, OPT_TRANSMIT_BINARY)
                        + T::MakeClientSb(OPT_NAWS, nawsParam)
                        + T::MakeCmd(CMD_WONT, OPT_TSPEED);
        return res; 
    }

    // ----------------------------------------

    QString TestData::ClientUserOutputData()
    {
        return QString(CTextBegin 
                       + CTextMid 
                       + CTextEnd 
                       + CLocation.AsQString() 
                       + "Terminal type is "
                       + CTerminalType);
    }

    // ----------------------------------------

    QString TestData::ClientUserData()
    {
        QString res = "data";

        return res;
    }

    // ----------------------------------------

    QString TestData::ServerShellData()
    {
        QString res = "data";

        return res;
    }

    // ----------------------------------------

    QString TestData::DataCheckSuccessful()
    {
        return QString("*Data was checked successful*");
    }

    // ----------------------------------------

    QString TestData::CheckClientInterface()
    {
        return QString("*Check client interface*");
    }

    // ----------------------------------------

    QString TestData::FinishIndData()
    {
        return QString("*Finish indication data*");
    }

    // ----------------------------------------

    QString TestData::FinishReqData()
    {
        return QString("*Finish request data*");
    }

    // ----------------------------------------

    QString TestData::FinishConfData()
    {
        return QString("*Finish confirm data*");
    }

} // namspace TelnetTest 

