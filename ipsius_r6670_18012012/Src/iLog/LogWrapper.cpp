#include "stdafx.h"

#include "ProjConfigLocal.h"

#include "Utils/IBasicInterface.h"
#include "Utils/UtilsDateTime.h"
#include "Utils/StringUtils.h"
#include "iLog/LogWrapperLoggers.h"

#include "LogTimestamp.h"
#include "LogWrapper.h"
#include "LogCommonImpl.h"
#include "LogWrapperInterfaces.h"
#include "LogWrapperLibrary.h"

using Utils::PadLeft;
using iLogCommon::LogStringConvert;
using iLogCommon::LogStringStream;
using iLogCommon::LogString;

// -------------------------------------------

namespace 
{
	const char *CPathSeparator = "/";
};

namespace iLogW
{

	void EndRecord() {}
	
    class LogStoreInternal : 
        public ILogSessionToLogStore,
        public iLog::ILogStoreStrategy<iLogCommon::LogRecord>
    {
        iLogW::ILogStoreStrategy *m_pStrategy;
        iLogCommon::LogStore m_logStore;

    // ILogStoreStrategy impl
    private:

        bool OnNewRecord(LogStoreAccess &logStore, iLogCommon::LogRecord &record)
        {
            if (m_pStrategy != 0) return m_pStrategy->OnNewRecord(logStore, record);
            return true;
        }

        void OnFinalize(LogStoreAccess &logStore)
        {
            if (m_pStrategy != 0) return m_pStrategy->OnFinalize(logStore);
        }

    // ILogSessionToLogStore impl
    public:

        iLogCommon::LogStore& getLogStore()
        {
            return m_logStore;
        }

    public:

        LogStoreInternal(iLogW::ILogStoreStrategy *pStrategy,
            bool useRecordsReorder,
            bool dublicateSessionNamesDetection) : 
            m_pStrategy(pStrategy), 
            m_logStore(this, useRecordsReorder, dublicateSessionNamesDetection)
        {
        }

    };

    // -------------------------------------------------------------------

    LogStore::LogStore(iLogW::ILogStoreStrategy *pStrategy,
        bool useRecordsReorder,
        bool dublicateSessionNamesDetection )
    {
        m_logStore.reset( new LogStoreInternal(pStrategy, useRecordsReorder, dublicateSessionNamesDetection) );
    }

    ILogSessionToLogStore& LogStore::getSessionInterface()
    {
        return *(m_logStore.get());
    }

    /*
    void LogStore::Save(LogBasicStream &stream)
    {
        DefaultFormater df;
        FormatedOutput fo(stream, df);

        Save(fo);
    } */

    void LogStore::Save(LogBasicStream &stream)
    {
        LogOutput outputList;
        outputList.Register(&stream);

        DefaultSelector selector;

        Save(outputList, selector);
    }


    void LogStore::Save(LogOutput &output, ILogSelector &selector)
    {
        typedef std::list<iLogCommon::LogRecord*> Records;

        // select records
        Records result;
        m_logStore->getLogStore().Select(selector.getSelector(), result);

        // process
        Records::iterator i = result.begin();

        while(i != result.end())
        {
            iLogCommon::LogRecord *p = (*i);
            output.Process(*p);

            ++i;
        }

    }

    LogStore::~LogStore()
    {
        // nothing, for scoped_ptr
    }

    iLog::LogStat LogStore::GetStat( bool withClear /*= false*/ )
    {
        return m_logStore->getLogStore().GetStat(withClear);
    }

    void LogStore::DumpSessionNames(std::vector<std::string> &dump)
    {
        return m_logStore->getLogStore().DumpSessionNames(dump);
    }

    // -------------------------------------------------------------------

    // NULL implementation
    class LogSessionInternal : 
        public ILoggerToSession, 
        public iLog::ILogSessionSync<iLogCommon::LogRecord>
    {
        const LogString m_name;
        bool m_logActive;
        LogRecordTag m_tag;

    // ILogSessionSync<> impl
    private:

        bool DoSyncWithStore(iLogCommon::LogRecord *pRecord) 
        {
            return true;
        }

    // ILoggerToSession impl
    public:

        void AddRecord(const LogString &data, LogRecordTag recordKind)
        {
            // nothing
        }

    public:

        LogSessionInternal(iLogCommon::LogStore &store, const LogString &name, bool active, 
            LogSessionProfile &profile) : 
            m_name(name), m_logActive(active)
        {
        }

        bool getLogActive() const
        {
            return m_logActive;
        }

        void setLogActive(bool value)
        {
            m_logActive = value;
        }

        virtual void Sync()  // nothing
        {
        }

        virtual LogRecordTag RegisterNewRecordKind(LogString name)
        {
            return m_tag;
        }

        virtual const LogString& Name() const
        {
            return m_name;
        }

    };

    // -------------------------------------------------------------------

    class LogSessionInternalImpl : public LogSessionInternal
    {
        iLogCommon::LogSession m_session;
        LogSessionProfile &m_profile;

        static iLog::Timestamp::CaptureModeType GetTimerCaptureMode(const LogSessionProfile &profile)
        {
            return (profile.getUseTimerCapture()) ? 
                iLog::Timestamp::CM_Full : 
                iLog::Timestamp::CM_None;
        }


    // ILogSessionSync<> impl
    private:

        bool DoSyncWithStore(iLogCommon::LogRecord *pRecord) 
        {
            ESS_ASSERT(pRecord != 0);

            return m_profile.getProcessInterface().DoSyncWithStore(*pRecord);
        }

    // ILoggerToSession impl
    public:

        void AddRecord(const LogString &data, LogRecordTag recordKind)
        {
            if (!getLogActive()) 
            {
                m_session.LogToSession( LogStringConvert::To("Warning! Write to non-active session!") );
                return;
            }

            m_session.LogToSession(data, recordKind);
        }

    public:

        LogSessionInternalImpl(iLogCommon::LogStore &store, const LogString &name, bool active, 
            LogSessionProfile &profile) : 
            LogSessionInternal(store, name, active, profile),
            m_session(store, name, GetTimerCaptureMode(profile), this), m_profile(profile)
        {
        }

        void Sync()  // override
        {
            m_session.DropSessionToStore();
        }

        LogRecordTag RegisterNewRecordKind(LogString name) // override
        {
            return m_session.CreateRecordKind(name);
        }

        const LogString& Name() const // override
        {
            return m_session.getName();
        }

    };

    // -------------------------------------------------------------------
	LogSession::LogSession(Utils::SafeRef<ILogCreatorHelper> creator,
		const LogString &name,
		Utils::SafeRef<LogSession> parrentSession)
    {
		ESS_ASSERT(!creator.IsEmpty());
		m_creator = creator;
		m_parrentSession = parrentSession;
        
		m_debug = false;        
        if (m_debug) 
		{
			std::cout << "Create log session <" 
				<< LogStringConvert::From(name) 
				<< ">" << std::endl;
		}
        typedef boost::mpl::if_c<ProjConfig::CfgLogWrapper::CDisableLogging, 
            LogSessionInternal, LogSessionInternalImpl>::type LogType; 
        
        LogSessionInternal *p = 
            new LogType(creator->getSessionInterface().getLogStore(), name, true, creator->getLogSessionProfile());
        m_internal.reset(p);
    }

	// -------------------------------------------------------------------

	LogSession::LogSession(ILogSessionToLogStore &store, 
		const LogString &name, LogSessionProfile &profile)
	{
		m_debug = false;        
		if (m_debug) 
		{
			std::cout << "Create log session <" 
				<< LogStringConvert::From(name) 
				<< ">" << std::endl;
		}
		typedef boost::mpl::if_c<ProjConfig::CfgLogWrapper::CDisableLogging, 
			LogSessionInternal, LogSessionInternalImpl>::type LogType; 

		LogSessionInternal *p = 
			new LogType(store.getLogStore(), name, true, profile);
		m_internal.reset(p);
	}

	// -------------------------------------------------------------------

    ILoggerToSession &LogSession::getLoggerInterface()
    {
        return *m_internal;
    }

	Utils::SafeRef<ILogSessionCreator> LogSession::LogCreator() 
	{
		return this;
	}

	Utils::WeakRef<ILogSessionCreator&> LogSession::LogCreatorWeakRef()
	{
		return m_selfRefHost.Create<ILogSessionCreator&>(*this);
	}

    void LogSession::Sync()
    {
        m_internal->Sync();
    }

    void LogSession::FlushStream()
    {
        ESS_ASSERT( LogActive() );

        LogRecordTag tag = m_tag;
        m_tag = LogRecordTag();

        if (!m_stream.str().size()) return;

        Add(m_stream.str(), tag);
        m_stream.str(iLogCommon::LogStringConvert::To(""));
        m_stream.clear();
    }

	LogString LogSession::NameChildSession(const LogString &name) const
	{
		LogString fullName(Name());

		if (!fullName.empty()) fullName += iLogCommon::LogStringConvert::To(CPathSeparator);

		return fullName + name;
	}

    bool LogSession::LogActive() const
    {
		return 	m_traceInd && (m_parrentSession.IsEmpty() || m_parrentSession->LogActive());
    }
    
    void LogSession::LogActive(bool value)
    {
        m_traceInd.Set(value);
    }

	bool LogSession::LogActive(const LogRecordTag &tag) const
	{
		return 	tag.IsActive() && LogActive();
	}

	bool LogSession::IsLocalActive() const
	{
		return m_traceInd;
	}

    LogRecordTag LogSession::RegisterRecordKind(const LogString &name, bool isActive)
    {
        LogRecordTag tag = m_internal->RegisterNewRecordKind(name);
		if(isActive) tag.Activate(isActive);
		return tag;
    }

    LogRecordTag LogSession::RegisterRecordKindStr(const std::string &name, bool isActive)
    {
        return RegisterRecordKind( iLogCommon::LogStringConvert::To(name), isActive);
    }

    void LogSession::Add(const LogString &data, LogRecordTag recordKind)
    {
        m_internal->AddRecord(data, recordKind);
    }

    LogSession::~LogSession()
    {
        if (m_debug) std::cout << "Destroy log session <" << NameStr() << ">" << std::endl;
        
        if (LogActive()) FlushStream();

		if (getCountRef() != 0)
		{
			std::ostringstream oss;
			oss << "~LogSession " << getCountRef() 
				<< "Name  " << NameStr();
			ESS_HALT( oss.str() );        
		}
	}

    const LogString &LogSession::Name() const
    {
        return m_internal->Name();
    }

	std::string LogSession::NameStr() const
	{
		return iLogCommon::LogStringConvert::From( Name() );
	}

    // -------------------------------------------------------------------

    class LogSessionProfileInternal : public ILogSessionSyncStrategy
    {
        bool m_useTimestamp;
        LogOutput *m_pOutput;
        ILogSessionSyncStrategy *m_pSyncStrategy;

    // ILogSessionSyncStrategy impl
    private:

        bool DoSyncWithStore(iLogCommon::LogRecord &record)
        {
            if (m_pOutput != 0)
            {
                m_pOutput->Process(record);
            }

            return (m_pSyncStrategy == 0) ? 
                true : m_pSyncStrategy->DoSyncWithStore(record);
        }

    public:

        LogSessionProfileInternal(bool useTimestamp, LogOutput *pOutput, 
            ILogSessionSyncStrategy *pSyncStrategy) : 
            m_useTimestamp(useTimestamp), m_pOutput(pOutput), m_pSyncStrategy(pSyncStrategy)
        {
        }
		void setUseTimerCapture( bool par)
		{
			m_useTimestamp = par;
		}
        bool getUseTimerCapture() const
        { 
            return m_useTimestamp;
        }

    };

    // -------------------------------------------------------------------


    LogSessionProfile::LogSessionProfile(bool useTimestamp, LogOutput *pOutput, 
        ILogSessionSyncStrategy *pSyncStrategy)
    {
        LogSessionProfileInternal *p = new 
            LogSessionProfileInternal(useTimestamp, pOutput, pSyncStrategy);

        m_pimpl.reset(p);
    }

    LogSessionProfile::~LogSessionProfile()
    {
        // nothing
    }

	void LogSessionProfile::setUseTimerCapture(bool par)
	{ 
		return m_pimpl->setUseTimerCapture(par);
	}

    bool LogSessionProfile::getUseTimerCapture() const
    { 
        return m_pimpl->getUseTimerCapture();
    }

    ILogSessionSyncStrategy& LogSessionProfile::getProcessInterface()
    {
        return *m_pimpl;
    }

    // -------------------------------------------------------------------

    LoggerTextStream::~LoggerTextStream()
    {
        m_session.AddRecord(m_stream.str(), m_kind);
    }

    // -------------------------------------------------------------------

    LoggerStringStream::~LoggerStringStream()
    {
        std::string s = m_stream.str();
        m_session.AddRecord(LogStringConvert::To(s), m_kind);
    }


}  // namespace iLogW

