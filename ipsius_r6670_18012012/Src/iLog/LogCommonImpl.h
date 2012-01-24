#ifndef __LOGCOMMONIMPL__
#define __LOGCOMMONIMPL__

#include "stdafx.h"
#include "Utils/ThreadContext.h"
#include "Utils/StringUtils.h"
// #include "Utils/StringUtilsW.h"
#include "Platform/Platform.h"
#include "iLog/LogCommonImplHelpers.h"
#include "LogRecordT.h"
#include "LogSessionT.h"
#include "LogStoreT.h"
#include "LogTimestamp.h"

/*
     Реализация конкретных классов на основе шаблонов.
     Данные лога -- std::wstring
     Многопоточности на основе платформы.     
*/

namespace iLogCommon
{
    using namespace iLog;

    struct MultithreadLockStrategy
    {
        typedef Platform::Mutex Mutex;
        typedef Platform::MutexLocker Locker;
    };

    // -----------------------------------------------

    class Convert
    {
        static void Dummi();

    public:

        static LogString CharToData(const char *p)
        {
            Dummi();

            return iLogCommon::LogStringConvert::To(std::string(p));
        }   

        static std::string DataToString(const LogString &s)
        {
            return iLogCommon::LogStringConvert::From(s);
        }

    };

    // эта секция должна быть синхронизирована с инстанцированием в .cpp файле
    typedef LogRecordT<Timestamp, LogString, Convert>       LogRecord;
    typedef LogStoreT<LogRecord, MultithreadLockStrategy>   LogStore;
    typedef LogSessionT<LogRecord, Utils::ThreadContext>    LogSession;

    // -----------------------------------------------
        
    class LoggerTextStream
    {
        iLogCommon::LogStringStream m_stream;
        ILogSession<LogRecord> &m_session;

        bool m_useKind;
        LogRecord::RecordTag m_kind;

    public:
        LoggerTextStream(ILogSession<LogRecord> &session) 
            : m_session(session), m_useKind(false)
        {
        }

        LoggerTextStream(ILogSession<LogRecord> &session, LogRecord::RecordTag &kind) 
            : m_session(session), m_useKind(true), m_kind(kind)
        {
        }

        ~LoggerTextStream()
        {
            if (m_useKind)
            {
                m_session.LogToSession(m_stream.str(), m_kind);
            }
            else
            {
                m_session.LogToSession(m_stream.str());
            }
            
        }

        iLogCommon::LogStringStream& out() { return m_stream; }

    };


}  // namespace iLogCommon


#endif

