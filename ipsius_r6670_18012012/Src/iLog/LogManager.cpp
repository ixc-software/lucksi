#include "stdafx.h"

#include "LogWrapperLibrary.h"
#include "LogManager.h"

namespace iLogW
{
    struct LogManager::Impl : boost::noncopyable,
        ILogStoreStrategy
    {
	public:
		std::string m_pathSep;
		int  m_storeMaxRecords;
		int  m_storeCleanTo;
        LogStore m_store;
        LogOutput m_output;
        SessionSyncStrategy m_sync;
        LogSessionProfile m_profile;
        LogOutputManager m_outputManager;
	// Log Manager Adapter to old interface for Dss1
    public:

        LogStore  &Store()    { return m_store;   }
        LogSessionProfile &Profile()       { return m_profile;   }

    // ILogStoreStrategy impl
    private:
        bool OnNewRecord(LogStoreAccess &logStore, iLogCommon::LogRecord &record)
        {
            // output
            m_output.Process(record);

            // limit records in LogStore
            if (m_storeMaxRecords > 0)
            {
                if (logStore.StoredRecords().size() > m_storeMaxRecords)
                {
                    logStore.StoreOnlyLast(m_storeCleanTo);
                }
            }

            return (m_storeMaxRecords == 0) ? false : true;
        }

        void OnFinalize(LogStoreAccess &logStore)
        {
            // ...
        }

    public:
		Impl(const LogSettings &setting) : 
			m_pathSep(setting.PathSeparator()),
			m_storeMaxRecords(setting.StoreRecordsLimit()),
			m_storeCleanTo(setting.StoreCleanTo()),
			m_store(this, setting.UseRecordsReorder(), setting.DublicateSessionNamesDetection()),
			m_sync(setting.CountSyncroToStore(), setting.TimeoutSyncroToStore()),
			m_profile(setting.TimestampInd(), 0, &m_sync),
			m_outputManager(m_output, boost::shared_ptr<ILogRecordFormater>(), setting.out())
		{
		}
    };

    // ---------------------------------------------------------------

    LogManager::LogManager(const LogSettings &setting) : 
		m_impl(new Impl(setting))
    {}

	// ---------------------------------------------------------------

    LogManager::LogManager(boost::shared_ptr<ILogRecordFormater> formater,
        const LogSettings &setting) : 
		m_impl(new Impl(setting))
    {
		m_impl->m_outputManager.ChangeFormater(formater);
	}

	// ---------------------------------------------------------------

    LogManager::~LogManager(){}

	// ---------------------------------------------------------------

    LogOutputManager &LogManager::Output()	{	return m_impl->m_outputManager;	}
	const LogOutputManager &LogManager::Output() const  {	return m_impl->m_outputManager;	}

	// ILogSessionCreator

	LogSession *LogManager::CreateSessionExt(const LogString &name, 
		bool isActive)
	{
		LogSession *log = new LogSession(this, name);
		if(isActive) log->LogActive(isActive);
		return log;
	}

	// ---------------------------------------------------------------
    // ILogCreatorHelper

    ILogSessionToLogStore &LogManager::getSessionInterface()
    {
        return m_impl->Store().getSessionInterface();
    }

	// ---------------------------------------------------------------

    LogSessionProfile &LogManager::getLogSessionProfile()
    {
        return m_impl->Profile();
    }

	// ---------------------------------------------------------------

	bool LogManager::TimestampInd() const
	{
        return m_impl->Profile().getUseTimerCapture();
	}

	// ---------------------------------------------------------------

	void LogManager::TimestampInd(bool val)
	{
        m_impl->Profile().setUseTimerCapture(val);
	}

	// ---------------------------------------------------------------------

	int LogManager::CountToSync() const
	{
		return m_impl->m_sync.CountToSync();
	}

	// ---------------------------------------------------------------------

	void LogManager::CountToSync(int countToSync)
	{
		m_impl->m_sync.CountToSync(countToSync);
	}

	// ---------------------------------------------------------------------

	int LogManager::TimeoutToSync() const
	{
		return m_impl->m_sync.TimeoutToSync();
	}

	// ---------------------------------------------------------------------

	void LogManager::TimeoutToSync(int timeoutToSync)
	{
		m_impl->m_sync.TimeoutToSync(timeoutToSync);
	}

	// ---------------------------------------------------------------

	int LogManager::StoreRecordsLimit() const
	{
		return m_impl->m_storeMaxRecords;
	}

	// ---------------------------------------------------------------

	void LogManager::StoreRecordsLimit(int val)
	{
		m_impl->m_storeMaxRecords = val;
		if (m_impl->m_storeMaxRecords > 0) ESS_ASSERT(m_impl->m_storeCleanTo < m_impl->m_storeMaxRecords);
	}

	// ---------------------------------------------------------------

	int LogManager::StoreCleanTo() const
	{
		return m_impl->m_storeCleanTo;
	}

	// ---------------------------------------------------------------

	void LogManager::StoreCleanTo(int val)
	{
		m_impl->m_storeCleanTo = val;
		if (m_impl->m_storeMaxRecords > 0) ESS_ASSERT(m_impl->m_storeCleanTo < m_impl->m_storeMaxRecords);
	}

	// ---------------------------------------------------------------

	std::string LogManager::PathSeparator() const
	{
		return m_impl->m_pathSep;
	}

	// ---------------------------------------------------------------

	void LogManager::PathSeparator(const std::string &val)
	{
		ESS_ASSERT(val.size() > 0);
		m_impl->m_pathSep = val;
	}

    // ---------------------------------------------------------------

    iLog::LogStat LogManager::GetStat( bool withClear /*= false*/ )
    {
        return m_impl->Store().GetStat(withClear);
    }

    // ---------------------------------------------------------------

    void LogManager::DumpSessionNames(std::vector<std::string> &dump)
    {
        m_impl->Store().DumpSessionNames(dump);
    }

};
