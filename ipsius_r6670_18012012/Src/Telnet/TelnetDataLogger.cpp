
#include "stdafx.h"

#include "TelnetDataLogger.h"
#include "Utils/QtHelpers.h"
#include "Utils/ErrorsSubsystem.h"
#include "Platform/PlatformTypes.h"

using namespace Platform;

// --------------------------------------------

namespace 
{
    byte CCmdByteArray[] = {
        240, 241, 242, 243, 244, 
        245, 246, 247, 248, 249, 
        250, 251, 252, 253, 254, 255
    };

    QString CCmdStrArray[] = {
        "SE ", "NOP ", "DM ", "BREAK ", "IP ", 
        "AO ", "AYT ", "EC ", "EL ", "GA ", 
        "SB ", "WILL ", "WONT ", "DO ", "DONT ", "IAC "
    };

    // ---------------------------------

    byte COptByteArray[] = {
        0, 1, 3, 5, 24, 
        31, 32, 33, 35, 36, 
        39, 255
    };

    QString COptStrArray[] = {
        "TRANSMIT_BINARY ", "ECHO ", "SGA ", "STATUS ", "TTYPE ", 
        "NAWS ", "TSPEED ", "LFLOW ", "XDISPLOC ", "OLD_ENVIRON ", 
        "NEW_ENVIRON ", "EXTOPL "
    };

    // ---------------------------------

    byte CSuboptByteArray[] = { 0, 1, 2 };

    QString CSuboptStrArray[] = { "IS ", "SEND ", "INFO " }; 

    // ---------------------------------

    enum 
    {
        CCmdIndex = 1,
        COptIndex = 2,
        CSuboptIndex = 3,

        CNumberBase = 16,
    };

    // ---------------------------------

    QString CInputMarker = "\n|>> ";
    QString COutputMarker = "\n<<| ";
    QString CSeparator = " ";
    QString CCmdBegin = "[ ";
    QString CCmdEnd = "]";

    // ---------------------------------

    QString NumToStr(byte num)
    {
        QString res;
        res.setNum(num, CNumberBase);
        res += CSeparator;

        return res;
    }

    // ---------------------------------
    // unify telnet specifics and they word equivalent
    class TelnetSpecCharLists
    {
        std::vector<byte> m_cmdByteArray;
        std::vector<QString> m_cmdStrArray;

        std::vector<byte> m_optByteArray;
        std::vector<QString> m_optStrArray;

        std::vector<byte> m_suboptByteArray;
        std::vector<QString> m_suboptStrArray;

        QString FindEquivalentCmd(byte what)
        {
            // if not found
            QString res(NumToStr(what));
            
            for (size_t i = 0; i <m_cmdByteArray.size(); ++i)
            {
                if (m_cmdByteArray.at(i) == what)
                {
                    res = m_cmdStrArray.at(i);
                    break;
                }
            }

            return res;
        }

        QString FindEquivalentOpt(byte what)
        {
            // if not found
            QString res(NumToStr(what));

            for (size_t i = 0; i < m_optByteArray.size(); ++i)
            {
                if (m_optByteArray.at(i) == what)
                {
                    res = m_optStrArray.at(i);
                    break;
                }
            }

            return res;
        }

        QString FindEquivalentSubOpt(byte what)
        {
            // if not found
            QString res(NumToStr(what));

            for (size_t i = 0; i < m_suboptByteArray.size(); ++i)
            {
                if (m_suboptByteArray.at(i) == what)
                {
                    res = m_suboptStrArray.at(i);
                    break;
                }
            }

            return res;
        }

    public:
        TelnetSpecCharLists()
        : m_cmdByteArray(CCmdByteArray, 
                         CCmdByteArray + (sizeof(CCmdByteArray) / sizeof(byte))),
          m_cmdStrArray(CCmdStrArray, 
                        CCmdStrArray + (sizeof(CCmdStrArray) / sizeof(QString))),
          m_optByteArray(COptByteArray, 
                         COptByteArray + (sizeof(COptByteArray) / sizeof(byte))),
          m_optStrArray(COptStrArray, 
                         COptStrArray + (sizeof(COptStrArray) / sizeof(QString))),
          m_suboptByteArray(CSuboptByteArray, 
                         CSuboptByteArray + (sizeof(CSuboptByteArray) / sizeof(byte))),
          m_suboptStrArray(CSuboptStrArray, 
                         CSuboptStrArray + (sizeof(CSuboptStrArray) / sizeof(QString)))
        {}

        enum CharType
        {
            Cmd,
            Opt,
            SubOpt,
            VarOpt,
            ExtOpt,
        };

        QString FindEquivalent(byte what, CharType type)
        {
            ESS_ASSERT(m_cmdByteArray.size() == m_cmdStrArray.size());
            ESS_ASSERT(m_optByteArray.size() == m_optStrArray.size());
            ESS_ASSERT(m_suboptByteArray.size() == m_suboptStrArray.size());

            QString res;

            switch (type)
            {
            case (Cmd):
                res = FindEquivalentCmd(what);
                break;

            case (Opt):
                res = FindEquivalentOpt(what);
                break;

            case (SubOpt):
                res = FindEquivalentSubOpt(what);
                break;

            case (VarOpt):
                break;

            case (ExtOpt):
                break;

            default:
                ESS_ASSERT(0 && "Uknown char type");
            }

            return res;
        }
    };

    // ----------------------------------------

    QString Cmd_2bytesToString(TelnetSpecCharLists &tList, const QByteArray &data)
    {
        typedef TelnetSpecCharLists T;

        QString res = tList.FindEquivalent(data.at(CCmdIndex), T::Cmd);

        return res;
    }

    // ----------------------------------------

    QString SbToString(TelnetSpecCharLists &tList, const QByteArray &data)
    {
        typedef TelnetSpecCharLists T;

        int endOfCmdLen = 2;

        QString res = Cmd_2bytesToString(tList, data);
        res += tList.FindEquivalent(data.at(COptIndex), T::Opt);
        res += tList.FindEquivalent(data.at(CSuboptIndex), T::SubOpt);

        for (int i = (CSuboptIndex + 1); i < (data.size() - endOfCmdLen); ++i)
        {
            res += NumToStr(data.at(i));
        }

        QByteArray tmp(data.right(endOfCmdLen));

        res += Cmd_2bytesToString(tList, tmp);
        
        return res;
    }

    // ----------------------------------------


    
} // namespace

// ----------------------------------------

// TelnetLogParser implementation
namespace Telnet
{
    void TelnetDataLogger::AddParsedData(const QString &data, bool isInput, bool isTelnet)
    {
        if (isInput)
        {
            m_parsedInput.Add(data, isTelnet);
            m_parsedData.Add(data, isTelnet);
        }
        else
        { 
            m_parsedOutput.Add(data, isTelnet);
            m_parsedData.Add(data, isTelnet);
        }
    }

    // -----------------------------------------

    QString TelnetDataLogger::DataByOpt(const ParsedData &data, Options opt) const
    {
        QString res(LogHeader());

        switch (opt)
        {
        case (AllData):
            res += data.getAllData();
            break;

        case (OnlyTelnetData):
            res += data.getTelnetData();
            break;

        case (OnlyTextData):
            res += data.getUserData();
            break;

        default:
            ESS_ASSERT(0 && "Unknown option");
        }

        return res;
    }

    // -----------------------------------------

    QString TelnetDataLogger::LogHeader() const
    {
        QString res("\n\nLog name: ");
        res += m_logName;
        res += "\n";

        return res;
    }

    // -----------------------------------------

    void TelnetDataLogger::Add(const QByteArray &data, 
                               bool isInput, 
                               DataType dataType)
    {
        if (!m_enabled) return;

        typedef TelnetSpecCharLists T;
        T tList;

        QString res;
        if (isInput)
        {
            res = CInputMarker;
            m_rawInput += data;
        }
        else 
        {
            res = COutputMarker;
            m_rawOutput += data;
        }

        res += CCmdBegin;

        switch (dataType)
        {
        case (CMD_2b):
            res += Cmd_2bytesToString(tList, data);
            break;

        case (CMD_3b):
            res += Cmd_2bytesToString(tList, data);
            res += tList.FindEquivalent(data.at(COptIndex), T::Opt);
            break;

        case (SB):
            res += SbToString(tList, data);
            break;

        default:
            ESS_ASSERT(0 && "Unknown binary data type");
        }

        res += CCmdEnd;

        AddParsedData(res, isInput, true);
    }

    // -----------------------------------------

    void TelnetDataLogger::AddStr(const QString &data, bool isInput)
    {
        if (!m_enabled) return;

        QString res(CInputMarker);
        if (!isInput) res = COutputMarker;

        res += data;

        AddParsedData(res, isInput, false);
    }

    // -----------------------------------------

    void TelnetDataLogger::LogToScreenStr(const QString &data)
    {
        std::cout << data << std::endl; 
    }

    // -----------------------------------------

    void TelnetDataLogger::LogToScreen(const QByteArray &data)
    {
        for (int i = 0; i < data.size(); ++i)
        {
            std::cout << (int)(byte)data.at(i) << ",";
        }

        std::cout << std::endl;
    }

    // -----------------------------------------

    void TelnetDataLogger::ClearRaw() 
    {
        m_rawInput.clear();
        m_rawOutput.clear();
    }

    // -----------------------------------------

    void TelnetDataLogger::ClearParsed()
    {
        m_parsedData.Clear();
        m_parsedInput.Clear();
        m_parsedOutput.Clear();
    }

    // -----------------------------------------

    bool TelnetDataLogger::IsEmptyRaw() const
    {
        return (m_rawInput.isEmpty() && m_rawOutput.isEmpty());
    }

    // -----------------------------------------

    bool TelnetDataLogger::IsEmptyParsed() const
    {
        return (m_parsedData.IsEmpty()
                && m_parsedInput.IsEmpty()
                && m_parsedOutput.IsEmpty());
    }

} // namespace Telnet

