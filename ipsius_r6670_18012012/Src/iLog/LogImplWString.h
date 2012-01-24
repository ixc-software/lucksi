#ifndef __LOGIMPLWSTRING__
#define __LOGIMPLWSTRING__


#include "Utils/StringUtilsW.h"

#include "LogRecordT.h"
#include "LogSessionT.h"
#include "LogStoreT.h"
#include "LogTimestamp.h"

/*
     Реализация конкретных классов на основе шаблонов.
     Данные лога -- std::wstring
     Нет многопоточности.
     Лог просто выводиться на экран

*/

namespace iLogWStr
{
    using namespace iLog;

    class LockStrategy
    {
    };

    class LogManageStrategy
    {
    };

    // -----------------------------------------------

    class Convert
    {
    public:

        static std::wstring CharToData(const char *p)
        {
            std::wstring s;

            while(*p)
            {
                wchar_t c = *p++;
                s += c;
            }

            return s;
        }

        static std::string DataToString(const std::wstring &s)
        {
            return Utils::WStringToString(s);
        }

    };

    typedef LogRecordT<Timestamp, std::wstring, Convert> LogRecord;
    typedef LogStoreT<LogRecord> LogStore;
    typedef LogSessionT<LogRecord> LogSession;

    // -----------------------------------------------
    
    class LoggerStream
    {
        ILogSession<LogRecord> &m_session;
        std::wstringstream m_stream;

    public:

        LoggerStream(ILogSession<LogRecord> &session)
            : m_session(session) 
        {
        }

        ~LoggerStream()
        {
            m_session.LogToSession(m_stream.str());
        }

        std::wstringstream& out() { return m_stream; }

    };


} // namespace iLogWStr

#endif

