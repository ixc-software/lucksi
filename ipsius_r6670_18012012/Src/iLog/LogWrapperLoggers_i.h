#ifndef __LOGWRAPPERLOGGERS_I__
#define __LOGWRAPPERLOGGERS_I__

// DON'T INCLUDE THIS FILE DIRECTLY!
// For use in LogWrapperLoggers.h only

namespace iLogW
{
	
	class LoggerQTextStream : boost::noncopyable
    {
        ILoggerToSession &m_session;
        QString m_buff;
        QTextStream m_stream;
        LogRecordTag m_kind;

    public:
        LoggerQTextStream(LogSession &session, LogRecordTag recordKind = LogRecordTag())
            : m_session( session.getLoggerInterface() ), 
            m_stream(&m_buff), m_kind(recordKind)
        {
        }

        ~LoggerQTextStream();
        
        template<class T>
        LoggerQTextStream& operator << (const T& arg)
        {
            out() << arg;
            return *this;
        }
        
        QTextStream& out() { return m_stream; }
    };
	
}  // namespace iLogW

#endif

