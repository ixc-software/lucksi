#ifndef __TELNETUTILS__
#define __TELNETUTILS__

#include "Platform/PlatformTypes.h"

namespace Telnet
{
    using Platform::word;
    using Platform::byte;
    using namespace std;

    enum SendingMode
    {
        AsciiMode, // default; use codec if need
        BinaryMode,
        UnicodeMode,
    };

    //-----------------------------------
    // unify specific type convertions
    class TypeConvertion
    {
    public:
        static string DefaultCodecName();
        static QString ToAscii(const QByteArray &data, const string &codecName = "");
        static QString ToUnicode(const QByteArray &data);
        static QString ToBinary(const QByteArray &data);
        static QByteArray FromAscii(const QString &data,const string &codecName = "");
        static QByteArray FromUnicode(const QString &data);
        static QByteArray FromBinary(const QString &data);
        static word ToWord(byte hi, byte lo);
        /*
        static dword ToDword(const QByteArray &num);
        */

        static QByteArray ToQByteArray(word num);
    };

    // --------------------------------------------
    // wrap for windows params(using in messages)
    class WinSize
    {
        word m_width;
        word m_height;
    public:
        WinSize(word width = 800, word height = 600)
          : m_width(width), m_height(height) 
        {}

        word getWidth() { return m_width; }
        word getHeight() { return m_height; }

        void setWidth(word val) { m_width = val; }
        void setHeight(word val) { m_height = val; }
    };

    // -------------------------------------------------
    // wrap for telnet params(using in messages)
    class Location
    {
        QString m_host;
        QString m_displayNo;
        QString m_screenNo;
    public:
        Location(const QString &host = "127.0.0.0", 
                 const QString &displayNo = "0",
                 const QString &screenNo = "0")
          : m_host(host), m_displayNo(displayNo), 
            m_screenNo(screenNo)
        {}

        QString AsQString() const
        {
            QString loc = m_host;
            loc += ":";
            loc += m_displayNo;
            loc += ".";
            loc += m_screenNo;
            return loc;
        }
    };

    // --------------------------------------------
    // wrap for network params(using in messages)
    class RemoteAddress
    {
        string m_host;
        int m_port;

    public:
        RemoteAddress(const string &host, int port)
        : m_host(host), m_port(port)
        {}

        string getHost() { return m_host; }
        int getPort() { return m_port; }

        void setHost(const string &host) { m_host = host; }
        void setPort(int port) { m_port = port; }
    };

    // -----------------------------------------------
    // wrap for telnet params(using in messages)
    class CmdAndOpt
    {
        byte m_cmd;
        byte m_opt;

    public:
        CmdAndOpt(byte cmd, byte opt)
        : m_cmd(cmd), m_opt(opt)
        {}

        byte getCmd() { return m_cmd; }
        byte getOpt() { return m_opt; }

        void setCmd(byte cmd) { m_cmd = cmd; }
        void setOpt(byte opt) { m_opt = opt; }
    };

    // -----------------------------------------------

    /*
    class DataLogger
    {
        QString m_sentS;
        QByteArray m_sent;
        QByteArray m_received;
        
    public:
        DataLogger() {}

        void AddSent(const QByteArray &data) { m_sent += data; }
        void AddSentStr(const QString &data) { m_sentS += data; }
        void AddRecv(const QByteArray &data) { m_received += data; }

        void Clear()
        {
            m_sent.clear();
            m_sentS.clear();
            m_received.clear();
        }

        bool IsEmpty() 
        { 
            return ((m_sent.isEmpty()) 
                    && (m_sentS.isEmpty()) 
                    && (m_received.isEmpty()));
        }

        QByteArray getSent() { return m_sent; }
        QString getSentStr() { return m_sentS; }
        QByteArray getRecv() { return m_received; }
        
    };
    */
    
} // namespace Telnet

#endif

