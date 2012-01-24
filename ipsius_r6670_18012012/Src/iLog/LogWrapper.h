#ifndef __LOGWRAPPER__
#define __LOGWRAPPER__

#include "stdafx.h"
#include "iLogSessionCreator.h"
#include "LogSessionBody.h"
#include "iLog/LogCommonImplHelpers.h"
#include "iLog/LogStat.h"
#include "Utils/SafeRef.h"
#include "Utils/WeakRef.h"
#include "Utils/IBasicInterface.h"


/*
      »мплементаци€ набора классов дл€ логировани€, основанна€ на LogCommonImpl.h
      ќсновна€ задача -- "отв€затьс€" от шаблонов с целью увеличени€ скорости компил€ции
*/

namespace iLogW
{
    using iLogCommon::LogString;
    using boost::scoped_ptr;

    typedef iLog::RecordKind<LogString> LogRecordTag;

    // наход€тс€ в LogWrapperInterfaces.h, в этот .h не включать ни в коем случае - !!
    class ILogSessionToLogStore;
    class ILoggerToSession;
    class ILogRecordFormater;
    class ILogSaveToStream;
    class ILogSessionHandler;
    class IOutputStream;
    class ILogSelector;
    class ILogSessionSyncStrategy;
    class ILogStoreStrategy;

    class LogOutput;
    class LogBasicStream;

    // ---------------------------------------------------

    class LogStoreInternal;

    // Pimpl idiom for LogStore
    class LogStore : boost::noncopyable
    {
        scoped_ptr<LogStoreInternal> m_logStore;

    public:
        LogStore(iLogW::ILogStoreStrategy *pStrategy = 0, 
            bool useRecordsReorder = true,
            bool dublicateSessionNamesDetection = true);
        ~LogStore();

        ILogSessionToLogStore& getSessionInterface();

        iLog::LogStat GetStat(bool withClear = false);
        void DumpSessionNames(std::vector<std::string> &dump);

        void Save(LogBasicStream &stream);  // all records with default formater
        void Save(LogOutput &output, ILogSelector &selector);
    };

    // ---------------------------------------------------

    class LogSessionProfileInternal;

    // Settings for LogSession
    class LogSessionProfile : boost::noncopyable
    {
        scoped_ptr<LogSessionProfileInternal> m_pimpl;
    public:

        LogSessionProfile(bool useTimestamp = true, LogOutput *pOutput = 0, 
            ILogSessionSyncStrategy *pSyncStrategy = 0);
        ~LogSessionProfile();
        void setUseTimerCapture(bool par);
        bool getUseTimerCapture() const;
        ILogSessionSyncStrategy& getProcessInterface();
    };

    // ---------------------------------------------------

    class LogSessionInternal;

    void EndRecord();
    
	class ILogCreatorHelper : public Utils::IBasicInterface
	{	
	public:
		virtual ILogSessionToLogStore &getSessionInterface() = 0; 
		virtual LogSessionProfile &getLogSessionProfile() = 0; 
	};

    // Pimpl for LogSession
    class LogSession : boost::noncopyable,
		public virtual Utils::SafeRefServer,
		public ILogSessionCreator
    {
		Utils::WeakRefHost m_selfRefHost;
        scoped_ptr<LogSessionInternal> m_internal;
		
		Utils::SafeRef<ILogCreatorHelper> m_creator;
		Utils::SafeRef<LogSession> m_parrentSession;
		Utils::AtomicBool m_traceInd;

        iLogCommon::LogStringStream m_stream;
        LogRecordTag m_tag;


        // debug
        bool m_debug;
	
	private:
        void FlushStream();

		LogString NameChildSession(const LogString &name) const;
	// ILogSessionCreator
	private:
		LogSession *CreateSessionExt(const LogString &name, bool isActive)
		{
			LogSession *session = new LogSession(m_creator, 
				NameChildSession(name), this);
			session->LogActive(isActive);
			return session;
		}
	public:
        LogSession(Utils::SafeRef<ILogCreatorHelper> creator,
			const LogString &name,
			Utils::SafeRef<LogSession> parrentSession = Utils::SafeRef<LogSession>());
		
		// for ISDN and Rtp
		LogSession(ILogSessionToLogStore &store, 
			const LogString &name, 
			LogSessionProfile &profile);

        ~LogSession();
		
		ILoggerToSession &getLoggerInterface();
		
		Utils::SafeRef<ILogSessionCreator> LogCreator();

		Utils::WeakRef<ILogSessionCreator&> LogCreatorWeakRef();

	// Activation control
    public:
        bool LogActive() const;
        void LogActive(bool value);
        bool LogActive(const LogRecordTag &) const;
		bool IsLocalActive() const;

	// Stream
    public:
        LogSession& operator<< (const LogRecordTag &arg)
        {
            if (LogActive(m_tag)) FlushStream();

            m_tag = arg;

            return *this;
        }
        LogSession& operator<< (void (*f)())
        {
            ESS_ASSERT(f == &EndRecord);
			if (LogActive(m_tag)) FlushStream();

            return *this;
        }

        LogSession& operator<< (const std::string &arg)
        {
            if (LogActive(m_tag)) m_stream << iLogCommon::LogStringConvert::To(arg);
               
            return *this;
        }

        template<class T> 
	        LogSession& operator<< (const T &arg)
        {
            if (LogActive(m_tag)) m_stream << arg;
               
            return *this;
        }
	// Other
	public:
        const LogString &Name() const;
        std::string NameStr() const;
        void Sync();
        LogRecordTag RegisterRecordKind(const LogString &name, bool isActive = true);
        LogRecordTag RegisterRecordKindStr(const std::string &name, bool isActive = true);
        void Add(const LogString &data, LogRecordTag recordKind);
     };

}  // namespace iLogW

#include "iLog/LogWrapperEx_i.h"

#endif
