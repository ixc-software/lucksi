#ifndef _LOG_SETTINGS_H_
#define _LOG_SETTINGS_H_

#include "Utils/ErrorsSubsystem.h"
#include "iLog/LogOutSettings.h"

namespace iLogW
{
    class LogSettings
    {
    public:

        LogSettings() :
            m_isTimestampUsed(false),
            m_timeoutSyncroToStore(-1),
            m_countSyncroToStore(-1),
            m_storeMaxRecords(0),
            m_storeCleanTo(0),
            m_useRecordsReorder(true),
            m_dublicateSessionNamesDetection(true),
            m_pathSep("/")
          {}

        bool TimestampInd() const {  return m_isTimestampUsed; }
        void TimestampInd(bool par) { m_isTimestampUsed = par; }

        int  TimeoutSyncroToStore() const {  return m_timeoutSyncroToStore; }
        void TimeoutSyncroToStore(int  timeout) {  m_timeoutSyncroToStore = timeout; }

        int  CountSyncroToStore() const {    return m_countSyncroToStore; }
        void CountSyncroToStore(int  count) {    m_countSyncroToStore = count; }

        // -1 -- unlimited; 0 -- don't store
		void StoreRecordsLimit(int maxRecords)
		{
			m_storeMaxRecords = maxRecords;
			if (m_storeMaxRecords > 0) ESS_ASSERT(m_storeCleanTo < m_storeMaxRecords);
		}
        int StoreRecordsLimit() const { return m_storeMaxRecords; }
        
		void StoreCleanTo(int cleanTo)
		{
			m_storeCleanTo = cleanTo;
			if (m_storeMaxRecords > 0) ESS_ASSERT(m_storeCleanTo < m_storeMaxRecords);
		}
		int StoreCleanTo() const      { return m_storeCleanTo; }
        
        const LogOutSettings &out() const {   return m_out;   }
        LogOutSettings &out() {   return m_out;   }

        bool UseRecordsReorder() const { return m_useRecordsReorder; }
        void UseRecordsReorder(bool val) { m_useRecordsReorder = val; }

        bool DublicateSessionNamesDetection() const { return m_dublicateSessionNamesDetection; }
        void DublicateSessionNamesDetection(bool val) { m_dublicateSessionNamesDetection = val; }

        const std::string& PathSeparator() const { return m_pathSep; }
        void PathSeparator(const std::string &pathSep) 
        { 
            ESS_ASSERT(pathSep.size() > 0);
            m_pathSep = pathSep; 
        }

    private:

        LogOutSettings m_out;

        bool m_isTimestampUsed;         // Timestamp is used
        int  m_timeoutSyncroToStore;    // Timeout for synchronization with store
        int  m_countSyncroToStore;      // Count of records for synchronization
        int  m_storeMaxRecords;         // max records in LogStore, -1 for unlimited
        int  m_storeCleanTo;            // clean to this count if m_storeMaxRecords reached
        bool m_useRecordsReorder;
        bool m_dublicateSessionNamesDetection;
        std::string m_pathSep;

    };
}; 

#endif 


