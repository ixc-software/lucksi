#ifndef __TELNETDATALOGGER__
#define __TELNETDATALOGGER__

// #include "Utils/IStoragable.h"

namespace Telnet
{
    // logger for all input and output data
    // store raw and parsed data
    class TelnetDataLogger/* :
        public Utils::IStoragable*/
    {
        class ParsedData
        {
            QString m_telnetData;
            QString m_userData;
            QString m_allData;

        public:
            ParsedData() {}

            void Add(const QString &data, bool isTelnet)
            {
                if (isTelnet) m_telnetData += data;
                else m_userData += data;
                m_allData += data;
            }

            QString getTelnetData() const  { return m_telnetData; }
            QString getUserData() const { return m_userData; }
            QString getAllData() const { return m_allData; }

            void Clear()
            {
                m_telnetData.clear();
                m_userData.clear();
                m_allData.clear();
            }

            bool IsEmpty() const
            {
                return (m_telnetData.isEmpty() 
                        && m_userData.isEmpty() 
                        && m_allData.isEmpty());
            }
        };

        bool m_enabled;

        QByteArray m_rawInput;
        QByteArray m_rawOutput;

        ParsedData m_parsedInput;
        ParsedData m_parsedOutput;
        ParsedData m_parsedData; // input + ouput

        QString m_logName;

    public:
        enum Options
        {
            OnlyTextData,
            OnlyTelnetData,
            AllData,
        };

        enum DataType
        {
            CMD_2b, // 2 bytes
            CMD_3b,
            SB,
        };

    /*
    // override IStoragable
    private:
        void Load(QTextStream &stream)
        {
            // ...
        }

        void Save(QTextStream &stream) const
        {
            stream << m_parsedData.getAllData();
        }
    */

    private:
        void AddParsedData(const QString &data, bool isInput, bool isTelnet);
        QString DataByOpt(const ParsedData &data, Options opt) const;
        QString LogHeader() const;

    public:
        TelnetDataLogger(const QString &logName, bool enabled)
            : m_enabled(enabled), m_logName(logName)
        {
        }

        void Add(const QByteArray &data, bool isInput, DataType dataType);
        void AddStr(const QString &data, bool isInput);

        void ClearRaw();
        void ClearParsed();

        QByteArray getRawDataInput() const { return m_rawInput; }
        QByteArray getRawDataOutput() const  { return m_rawOutput; }
        QString getParsedDataInput(Options opt) const { return DataByOpt(m_parsedInput, opt); }
        QString getParsedDataOutput(Options opt) const { return DataByOpt(m_parsedOutput, opt); } 
        QString getParsedDataAll(Options opt) const { return DataByOpt(m_parsedData, opt); }

        bool IsEmptyRaw() const;
        bool IsEmptyParsed() const;

        static void LogToScreenStr(const QString &data);
        static void LogToScreen(const QByteArray &data);

    };

} // namespace Telnet


#endif
