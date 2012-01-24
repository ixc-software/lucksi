#include "stdafx.h"
#include "Utils/QtHelpers.h"
#include "Utils/StringList.h"
#include "Utils/ExeName.h"
#include "iCore/MsgThread.h"
#include "iLog/LogTimestamp.h"
#include "LogTest.h"
#include "LogCommonImpl.h"
// #include "LogImplQString.h"

// -----------------------------------------------------------

namespace
{
    using namespace iLog;
    using namespace iLogCommon;
    using iLogCommon::LogString;
    using Utils::QStringToWString;

    // obsolete simple example
    void _TestFn()
    {
        LogStore logStore;
        LogSession session(logStore, "test", iLog::Timestamp::CM_Full);

        {
            LoggerTextStream logger(session);
            logger.out() << "Test" << 15;
        }

        {
            QString name("Name");
            int value = 15;

            QString s = QString("%1 is %2").arg(name).arg(value);
            session.LogToSession( QStringToWString(s) );
        }

        {
            LogRecord::RecordTag tag = session.CreateRecordKind(L"New record kind");
            LoggerTextStream logger(session, tag);
            logger.out() << "Tagged test" << 15;
        }

        {
            LogSelector<LogRecord> selector;
            std::list<LogRecord*> result;
            logStore.Select(selector, result);
        }


    }

    // ---------------------------------------------------

    // стартегия для LogStore -- вывод записей на экран, сохранение в файл в конце работы
    class LogStoreStrategy : public ILogStoreStrategy<LogRecord>
    {
        bool m_useCout;

        static std::string FormatSimple(LogRecord &record)
        {
            std::wstring s = record.Data();
            return Utils::WStringToString(s);
        }

    // ILogStoreStrategy impl
    private:

        bool OnNewRecord(LogStoreAccess &logStore, LogRecord &record)
        {
            if (m_useCout)
            {
                std::cout << FormatSimple(record) << std::endl;
            }

            return true;
        }

        void OnFinalize(LogStoreAccess &logStore)
        {
            Utils::StringList sl;

            const RecordsList &list =  logStore.StoredRecords();
            RecordsList::const_iterator i = list.begin();
            while(i != list.end())
            {
                QString s = FormatExt(*i);
                sl.push_back(s);
                ++i;
            }
            std::string fileName = Utils::ExeName::GetExeDir();
            fileName += "log_test.txt";
            sl.SaveToFile(fileName.c_str());
        }

    public:

        LogStoreStrategy(bool useCout)
        {
            m_useCout = useCout;
        }

        QString FormatExt(LogRecord *pRecord)
        {            
            QString time = "<empty>";
            if (!pRecord->Timestamp().IsEmpty()) 
            {
                Utils::DateTime dt = pRecord->Timestamp().get();
                time = dt.time().ToString().c_str();
            }

            QString data = Utils::WStringToQString( pRecord->Data() );
            std::wstring sessionName = pRecord->Session();  // logStore.ResolveSessionTag( pRecord->getSessionTag() );
            QString session = Utils::WStringToQString(sessionName);
            QString recordKind = Utils::WStringToQString( pRecord->GetRecordKind() );

            return QString("%1 [%2] [%3] %4").arg(time, -10).arg(session, -20).arg(recordKind, -12).arg(data);
        }

    };

    // ---------------------------------------------------

    class ClassWithSession
    {
        class SessionSync : public ILogSessionSync<LogRecord>
        {
            int m_buffSize;
            int m_counter;

            bool DoSyncWithStore(LogRecord *pRecord)  // override
            {
                ++m_counter;

                if (m_counter >= m_buffSize)
                {
                    m_counter = 0;
                    return true;
                }

                return false;
            }

        public:
            SessionSync(int buffSize = 1) : m_buffSize(buffSize), m_counter(0) {}
        };

        SessionSync m_sync;
        LogSession m_session;
        LogRecord::RecordTag m_recordSpecialKind;
        int m_counter;

        void Write(std::wostringstream &s)
        {
            s << m_counter++ << " from " << m_session.getName();
        }

    public:

        ClassWithSession(LogStore &logStore, const char *pName, 
            Timestamp::CaptureModeType captureMode = Timestamp::CM_Full, int buffSize = 1)
            : m_sync(buffSize), m_session(logStore, pName, captureMode, &m_sync),
            m_counter(0)
        {
            m_recordSpecialKind = m_session.CreateRecordKind(L"special");
        }

        void Log()
        {
            LoggerTextStream logger(m_session);
            Write(logger.out());
        }

        void LogWithRecordKind()
        {
            LoggerTextStream logger(m_session, m_recordSpecialKind);
            Write(logger.out());
        }

        std::wstring getName() const { return m_session.getName(); }
    };

    // ---------------------------------------------------

    class TagFilter : public ISessionFilter<LogRecord::RecordData>
    {
        LogString m_selectDesc;

    // ISessionFilter<> impl
    private:

        bool SessionAccepted(const LogString &tagDesc)  // override
        {
            return (tagDesc == m_selectDesc);
        }

    public:

        TagFilter(LogString selectDesc) : m_selectDesc(selectDesc) {}

    };

    // ---------------------------------------------------

    void TestFn(bool useCout)
    {
        LogStoreStrategy strategy(useCout);
        LogStore logStore(&strategy);

        Timestamp ts;
        std::wstring oneName;

        {
            ClassWithSession one(logStore,    "One");
            ClassWithSession two(logStore,    "Two",   Timestamp::CM_Full, 3);
            ClassWithSession three (logStore, "Three", Timestamp::CM_None);

            ClassWithSession oneDublicate(logStore, "One");

            oneName = one.getName();

            // логирование
            for(int i = 0; i < 4; i++)
            {
                if (i == 1) ts.Capture();

                one.Log();

                if ((i & 1) == 0) two.Log(); 
                else two.LogWithRecordKind();

                three.Log();

                int delay = (i == 0) ? 1500 : 100;
                Platform::Thread::Sleep(delay);
            }
        }


        // выборка -- все записи сессии one после времени ts
        {
            LogTimestampFilter<LogRecord> tsFilter(ts, true);
            TagFilter tagFilter(oneName);
            LogSelector<LogRecord> selector(tsFilter, tagFilter);

            typedef std::list<LogRecord*> Records;

            Records result;
            logStore.Select(selector, result);

            if (useCout) 
            {
                std::cout << std::endl << std::endl << "Select" << std::endl;
            }                

            for(Records::iterator i = result.begin(); i != result.end(); ++i)
            {
                QString s = strategy.FormatExt(*i);

                if (useCout)
                {
                    std::cout << s << std::endl;
                }                
            }

            int i = 0; // dummi


        }

        if (useCout) 
        {
            std::cout << std::endl << std::endl;
        }                
    }


}  // namespace

/*

    Reference output:

    Dublicate desciption registred, One
    0 from One
    0 from Three
    1 from One
    1 from Three
    2 from One
    0 from Two
    1 from Two
    2 from Two
    2 from Three
    3 from One
    3 from Three
    3 from Two


    Select
    17:57:27.783 [One                 ] [<default>   ] 1 from One
    17:57:27.892 [One                 ] [<default>   ] 2 from One
    17:57:27.986 [One                 ] [<default>   ] 3 from One

*/

// -----------------------------------------------------------

namespace iLog
{

    void iLogTest2(bool useCout)
    {
        TestFn(useCout);
    }

}  // namespace iLog


