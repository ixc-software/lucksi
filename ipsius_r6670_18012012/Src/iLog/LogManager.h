#ifndef __LOGMANAGER__
#define __LOGMANAGER__

#include "stdafx.h"
#include "LogSettings.h"
#include "LogWrapper.h"
#include "LogManagerOutput.h"
#include "LogStat.h"

namespace iLogW
{
    class ILogRecordFormater;
    class LogOutput;


    class LogManager : boost::noncopyable, 
		public virtual Utils::SafeRefServer,
		public ILogSessionCreator,
		public ILogCreatorHelper
    {
    public:
        LogManager(const LogSettings &);
        ~LogManager();
       
        LogManager(boost::shared_ptr<ILogRecordFormater>,
                   const LogSettings &);

		Utils::SafeRef<ILogSessionCreator> LogCreator()
		{
			return this; 
		}

        LogOutputManager &Output();
        const LogOutputManager &Output() const ;
		
		bool TimestampInd() const;
		void TimestampInd(bool val);

		int CountToSync() const;
		void CountToSync(int countToSync);

		int TimeoutToSync() const;
		void TimeoutToSync(int timeoutToSync);

		int StoreRecordsLimit() const;
		void StoreRecordsLimit(int val);

		int StoreCleanTo() const;
		void StoreCleanTo(int val);

		std::string PathSeparator() const;
		void PathSeparator(const std::string &val);

        iLog::LogStat GetStat(bool withClear = false);
            
        // debug
        void DumpSessionNames(std::vector<std::string> &dump);
            
	// ILogSessionCreator
	public:
		LogSession *CreateSessionExt(const LogString &name, 
				bool isActive = false);		
	// ILogCreatorHelper
    public: // for ISDN
        ILogSessionToLogStore &getSessionInterface();
        LogSessionProfile &getLogSessionProfile();
    private:
		LogSession &RootSession();
    private:
        struct Impl;
        boost::scoped_ptr<Impl> m_impl;
    };

    void LogManagerExample();
};

#endif
