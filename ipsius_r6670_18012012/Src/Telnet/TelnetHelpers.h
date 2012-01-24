#ifndef __TELNETHELPERS__
#define __TELNETHELPERS__

#include "stdafx.h"
#include "TelnetCommands.h"
#include "TelnetOptionList.h"
#include "TelnetUtils.h"

#include "Utils/ErrorsSubsystem.h"


namespace Telnet
{
    using std::string;

    // class collecting all telnet specific convertions 
    // used by TelnetSession class
    class TelnetHelpers
    {
    public:
        static byte At(const QByteArray &array, int index)
        {
            return static_cast<byte>(array.at(index));

        }

        static void Append(QByteArray &array, byte val)
        {
            array.append(static_cast<char>(val));
        }

        static QByteArray DoubleIacs(const QByteArray &data)
        {
            QByteArray result;
            result.reserve(data.size());
            for (int i = 0; i < data.size(); ++i)
            {
                result += data.at(i);
                if (At(data,i ) == CMD_IAC) result += data.at(i);
            }
            return result;
        }

        static QByteArray CutDoubleIacs(const QByteArray &data)
        {
            QByteArray res = data;
            for (int i = 0; i < res.size(); ++i)
            {
                if ((At(res, i) == CMD_IAC)
                    && (At(res, (i + 1)) == CMD_IAC))
                {
                    res.remove(i, 1);
                }
            }
            return res;
        }

        static QByteArray MakeCmd(byte cmd, byte opt)
        {
            QByteArray res;
            Append(res, CMD_IAC);
            Append(res, cmd);
            Append(res, opt);

            return res;
        }

        static QByteArray MakeCmd(byte cmd)
        {
            QByteArray res;
            Append(res, CMD_IAC);
            Append(res, cmd);

            return res;
        }

        static QByteArray MakeServerSb(byte opt)
        {
            QByteArray res;
            Append(res, CMD_IAC);
            Append(res, CMD_SB);
            Append(res, opt);
            Append(res, SOPT_SEND);
            Append(res, CMD_IAC);
            Append(res, CMD_SE);

            return res;

        }

        static QByteArray MakeServerSb(byte opt, const QByteArray &params)
        {
            QByteArray res;
            Append(res, CMD_IAC);
            Append(res, CMD_SB);
            Append(res, opt);
            Append(res, SOPT_SEND);
            res += TelnetHelpers::DoubleIacs(params);
            Append(res, CMD_IAC);
            Append(res, CMD_SE);

            return res;
        }

        static QByteArray MakeClientSb(byte opt, const QByteArray &params)
        {
            QByteArray res;
            Append(res, CMD_IAC);
            Append(res, CMD_SB);
            Append(res, opt);

            if (opt != OPT_NAWS) Append(res, SOPT_IS);

            res += TelnetHelpers::DoubleIacs(params);
            Append(res, CMD_IAC);
            Append(res, CMD_SE);

            return res;
        }

        static QByteArray MakeClientSbEnvironParams(const string &varName,
                                                    const string &varValue, 
                                                    bool userVar = false)
        {
            byte opt = (userVar) ? VOPT_USERVAR : VOPT_VAR;
            QByteArray res;
            Append(res, opt);
            res += TelnetHelpers::DoubleIacs(QByteArray(varName.c_str()));
            Append(res, VOPT_VALUE);
            res += TelnetHelpers::DoubleIacs(QByteArray(varValue.c_str()));

            return res;
        }

        static QByteArray MakeXoptCmd(byte cmd, byte xopt)
        {
            QByteArray res;
            Append(res, CMD_IAC);
            Append(res, CMD_SB);
            Append(res, OPT_EXTOPL);
            Append(res, cmd);
            Append(res, xopt);
            Append(res, CMD_IAC);
            Append(res, CMD_SE);

            return res;
        }

        static QByteArray MakeXoptSb(byte xopt, const QByteArray &params)
        {
            QByteArray res;
            Append(res, CMD_IAC);
            Append(res, CMD_SB);
            Append(res, OPT_EXTOPL);
            Append(res, CMD_SB);
            Append(res, xopt);
            res += DoubleIacs(params);
            Append(res, CMD_SE);
            Append(res, CMD_IAC);
            Append(res, CMD_SE);

            return res;
        }

        static byte CaseOptionByMode(SendingMode mode)
        {
            byte res = 0;
            switch (mode)
            {
            case (BinaryMode):
                res = OPT_TRANSMIT_BINARY;
                break;

            case (UnicodeMode):
                res = XOPT_TRANSMIT_UNICODE;
                break;

            case (AsciiMode):
                res = XOPT_TRANSMIT_CODEC;
                break;

            default:
                ESS_ASSERT(0 && "Unknown sending mode!");
            }

            return res;
        }

        static SendingMode CaseModeByOpt(byte opt)
        {
            switch (opt)
            {
            case (OPT_TRANSMIT_BINARY):
                return BinaryMode;
                
            case (XOPT_TRANSMIT_UNICODE):
                return UnicodeMode;

            case (XOPT_TRANSMIT_CODEC):
                return AsciiMode;

            default:
                ESS_ASSERT(0 && "Unknown sending mode!");
            }
            
            return BinaryMode; // dummi
        }

        /*
        static void DisablePrevMode(TelnetOptionList &optList)
        {
            SendingMode mode = optList.getOwnerSendingMode();

            switch (mode)
            {
            case (BinaryMode):
                // disable Unicode && Codepage opions
                optList.Disable(XOPT_TRANSMIT_UNICODE);
                optList.Disable(XOPT_TRANSMIT_CODEC);
                break;

            case (UnicodeMode):
                // disable Binary && Codepage opt
                optList.Disable(OPT_TRANSMIT_BINARY);
                optList.Disable(XOPT_TRANSMIT_CODEC);
                break;

            case (AsciiMode):
                // disable Unicode && Binary mode
                optList.Disable(OPT_TRANSMIT_BINARY);
                optList.Disable(XOPT_TRANSMIT_UNICODE);
                break;

            default:
                ESS_ASSERT(0 && "Unknown sending mode!");
            }
        }
        */

        static void DisablePrevMode(TelnetOptionList &optList, byte opt)
        {
            switch (opt)
            {
            case (OPT_TRANSMIT_BINARY):
                // disable Unicode && Codepage opions
                optList.Disable(XOPT_TRANSMIT_UNICODE);
                optList.Disable(XOPT_TRANSMIT_CODEC);
                break;

            case (XOPT_TRANSMIT_UNICODE):
                // disable Binary && Codepage opt
                optList.Disable(OPT_TRANSMIT_BINARY);
                optList.Disable(XOPT_TRANSMIT_CODEC);
                break;

            case (XOPT_TRANSMIT_CODEC):
                // disable Unicode && Binary mode
                optList.Disable(OPT_TRANSMIT_BINARY);
                optList.Disable(XOPT_TRANSMIT_UNICODE);
                break;

            default:
                ESS_ASSERT(0 && "Unknown sending mode!");
            }
        }


        static bool IsModeChangingOpt(byte opt)
        {
            return ((opt == OPT_TRANSMIT_BINARY) 
                    || (opt == XOPT_TRANSMIT_UNICODE) 
                    || (opt == XOPT_TRANSMIT_CODEC)) 
                   ? true 
                   : false;
        }

        // Convert depend on sending mode
        static QByteArray ToQByteArray(const QString &data, 
                                       SendingMode mode,
                                       const string &codecName)
        {
            QByteArray res;
            switch (mode)
            {
            case (AsciiMode):
                res = TypeConvertion::FromAscii(data, codecName);
                break;

            case (BinaryMode):
                res = TypeConvertion::FromBinary(data);
                break;

            case (UnicodeMode):
                res = TypeConvertion::FromUnicode(data);
                break;

            default:
                ESS_ASSERT(0 && "Unknown sending mode!");
            }

            return res;
        }

        // Convert depend on sending mode
        static QString ToQString(const QByteArray &data, 
                                 SendingMode mode,
                                 const string &codecName)
        {
            QString res;
            switch (mode)
            {
            case (AsciiMode):
                res = TypeConvertion::ToAscii(data, codecName);
                break;

            case (BinaryMode):
                res = TypeConvertion::ToBinary(data);
                break;

            case (UnicodeMode):
                res = TypeConvertion::ToUnicode(data);
                break;

            default:
                ESS_ASSERT(0 && "Unknown sending mode!");
            }

            // to fix linux enter -- 0x0d 0x00           
            QChar endZero = QChar('\x00');
            if ((*(data.end() - 1) == endZero)
                && (*(res.end() - 1) != endZero))
            {
                res += endZero;
            }

            return res;
        }
    };

} // namespace Telnet

#endif
