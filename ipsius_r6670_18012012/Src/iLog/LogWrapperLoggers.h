#ifndef __LOGWRAPPERLOGGERS__
#define __LOGWRAPPERLOGGERS__

#include "iLog/LogWrapper.h"

namespace iLogW
{
	
    // Logger for "native" stream
    class LoggerTextStream : boost::noncopyable
    {
        ILoggerToSession &m_session;
        iLogCommon::LogStringStream m_stream;
        LogRecordTag m_kind;

        template<class TStream>
        static void WriteString(TStream &ss, const std::string &s)
        {
            for(size_t i = 0; i < s.size(); ++i)
            {
                ss << s[i];
            }
        }

        static void WriteString(std::ostringstream &ss, const std::string &s)
        {
            ss << s;
        }

    public:
        LoggerTextStream(LogSession &session, LogRecordTag recordKind = LogRecordTag())
            : m_session( session.getLoggerInterface() ), m_kind(recordKind)
        {
        }

        ~LoggerTextStream();
        
        template<class T>
        LoggerTextStream& operator << (const T& arg)
        {
            out() << arg;
            return *this;
        }

        // если LogString есть std::wstring, в поток нельзя записать std::string
        LoggerTextStream& operator << (const std::string &s)
        {
            WriteString(out(), s);
            return *this;
        }
        
        iLogCommon::LogStringStream& out() { return m_stream; }
    };
    
    // -----------------------------------------------------

    // Logger for std::ostringstream
    class LoggerStringStream : boost::noncopyable
    {
        ILoggerToSession &m_session;
        std::ostringstream m_stream;
        LogRecordTag m_kind;

    public:
        LoggerStringStream(LogSession &session, LogRecordTag recordKind = LogRecordTag())
            : m_session( session.getLoggerInterface() ), 
            m_kind(recordKind)
        {
        }

        ~LoggerStringStream();
        
        template<class T>
        LoggerTextStream& operator << (const T& arg)
        {
            out() << arg;
            return *this;
        }
        
        std::ostringstream& out() { return m_stream; }
    };
	
	
}  // namespace iLogW

// Platform depend loggers
#include "iLog/LogWrapperLoggers_i.h"

#endif

