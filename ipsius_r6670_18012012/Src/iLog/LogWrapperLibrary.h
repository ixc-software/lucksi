#ifndef __LOGWRAPPERLIBRARY__
#define __LOGWRAPPERLIBRARY__

/*
    �� � ���� ������ �� �������� ���� ���� � ������ .h �����!
    �� ����� �� ����� ������� ��������� ���������� �� iLog!

*/
#include "Platform/Platform.h"
#include "iLog/LogWrapperInterfaces.h"
#include "Utils/TimerTicks.h"

namespace iLogW
{
    using iLogCommon::LogString;

    // ---------------------------------------------------------------------

    // Default record to string formater
    class DefaultFormater : public ILogRecordFormater
    {

    // ILogRecordFormater impl
    public:

        void FormatRecord(iLogCommon::LogRecord &record, LogString &result)
        {
            using Utils::PadLeft;
            using iLogCommon::LogStringConvert;
            using iLogCommon::LogStringStream;
            using iLogCommon::LogString;
            
            LogString time;// = LogStringConvert::To("<empty>");
            if ( !record.Timestamp().IsEmpty() ) 
            {
                Utils::DateTime dt = record.Timestamp().get();
                time = LogStringConvert::To( dt.time().ToString() );
            }

            LogString data = record.Data();
            LogString session = record.Session();
            LogString recordKind = record.GetRecordKind();

            // QString("%1 [%2] [%3] %4").arg(time, -10).arg(session, -16).arg(recordKind, -12).arg(data)

            LogStringStream os;
            os << PadLeft(time, 10, ' ')
                << " [" << PadLeft(session, 16, ' ') << "] "
                << " [" << PadLeft(recordKind, 12, ' ') << "] "
                << data;

            result = os.str(); 
        }

    public:

        DefaultFormater( /* options... */ )
        {
        }

    };

    // ---------------------------------------------------------------------

    // Special record to string formater. Each messages formated as block. Blocks separated empty line.
    class MsgBlockFormater : public ILogRecordFormater
    {

    // ILogRecordFormater impl
    public:

        void FormatRecord(iLogCommon::LogRecord &record, LogString &result)
        {
            using Utils::PadLeft;
            using iLogCommon::LogStringConvert;
            using iLogCommon::LogStringStream;
            using iLogCommon::LogString;

            LogString time = LogStringConvert::To("<empty>");
            if ( !record.Timestamp().IsEmpty() ) 
            {
                Utils::DateTime dt = record.Timestamp().get();
                time = LogStringConvert::To( dt.time().ToString() );
            }

            LogString data = record.Data();
            LogString session = record.Session();
            LogString recordKind = record.GetRecordKind();

            LogStringStream os;
            os << "[Time: " << PadLeft(time, 10, ' ') << "]\n"
                << "[From:" << PadLeft(session, 10, ' ') << "]\n"
                << "[Kind:" << PadLeft(recordKind, 10, ' ') << "]\n"
                << data << "\n\n";

            result = os.str(); 
        }

    public:

        MsgBlockFormater( /* options... */ )
        {
        }

    };

    // ---------------------------------------------------------------------

    // Default selector -- select all records
    class DefaultSelector : public ILogSelector
    {
        iLog::LogSelector<iLogCommon::LogRecord> m_selector;

    // ILogSelector impl
    public:

        iLog::ILogSelector<iLogCommon::LogRecord>& getSelector()
        {
            return m_selector;
        }

    };

    // ---------------------------------------------------------------------

    class SessionSyncStrategy : public ILogSessionSyncStrategy
    {
        int m_countToSync;
        int m_timeoutToSync;

        Utils::TimerTicks m_timer;
        int m_recordsCounter;

    // ILogSessionSyncStrategy impl
    private:

        bool DoSyncWithStore(iLogCommon::LogRecord &record)
        {
            bool syncReq = false;

            m_recordsCounter++;

            // check by counter
            if (m_countToSync > 0)
            {
                if (m_recordsCounter >= m_countToSync)
                {
                    m_recordsCounter = 0;
                    syncReq = true;
                }
            }

            // check by timer
            if (m_timeoutToSync > 0)
            {
                if (m_timer.Get() >= Platform::dword(m_timeoutToSync))
                {
                    m_timer.Reset();
                    syncReq = true;
                }
            }

            return syncReq;
        }

    public:

        SessionSyncStrategy(int countToSync = 1, int timeoutToSync = -1)
            : m_countToSync(countToSync), m_timeoutToSync(timeoutToSync)
        {
            m_timer.Reset();
            m_recordsCounter = 0;
        }
		int CountToSync() const
		{
			return m_countToSync;
		}
		void CountToSync(int countToSync)
		{
			m_countToSync = countToSync;
		}
		int TimeoutToSync() const
		{
			return m_timeoutToSync;
		}
		void TimeoutToSync(int timeoutToSync)
		{
			m_timeoutToSync = timeoutToSync;
		}
    };

    // ---------------------------------------------------------------------

    // Basic log stream class
    class LogBasicStream
    {
        boost::shared_ptr<ILogRecordFormater> m_formater;

        virtual void Write(const iLogW::LogString &data) = 0;

    public:

        LogBasicStream(boost::shared_ptr<ILogRecordFormater> formater)
            : m_formater(formater)
        {
            if (m_formater.get() == 0)
            {
                m_formater.reset( new DefaultFormater() );
            }
        }

        virtual ~LogBasicStream()
        {
        }

        void Process(iLogCommon::LogRecord &record)
        {
            LogString s;
            m_formater->FormatRecord(record, s);

            Write(s);
        }


    };	

    // ---------------------------------------------------------------------

    // ������ FormatedOutput
    class LogOutput : boost::noncopyable
    {
        typedef std::vector<LogBasicStream*> List;
        List m_list;

    public:

        LogOutput()
        {
        }

        void Register(LogBasicStream *p)
        {
            ESS_ASSERT(p != 0);

            List::iterator i = std::find(m_list.begin(), m_list.end(), p);
            ESS_ASSERT(i == m_list.end());

            m_list.push_back(p);
        }

        void Unregister(LogBasicStream *p)
        {
            ESS_ASSERT(p != 0);

            List::iterator i = std::find(m_list.begin(), m_list.end(), p);
            ESS_ASSERT(i != m_list.end());

            m_list.erase(i);

        }

        void Process(iLogCommon::LogRecord &record)
        {
            for(List::iterator i = m_list.begin(); i != m_list.end(); ++i)
            {
                (*i)->Process(record);
            }
        } 


    };

    // ---------------------------------------------------------------------

    // Streaming to std::cout
    class LogCoutStream : public LogBasicStream
    {

    public:

        LogCoutStream(boost::shared_ptr<ILogRecordFormater> formater) : LogBasicStream(formater)
        {
        }

        void Write(const iLogW::LogString &data)  // override
        {
            std::cout << iLogCommon::LogStringConvert::From(data) << std::endl;
        }

    };
}  // namespace LogW

// Platform depend LogFileStream implementation
#include "iLog/LogFileStream_i.h"

#endif

