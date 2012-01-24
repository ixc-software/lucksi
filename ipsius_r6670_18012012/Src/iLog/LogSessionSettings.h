#ifndef _LOG_SESSION_SETTINGS_H_
#define _LOG_SESSION_SETTINGS_H_

#include "stdafx.h"

namespace Utils
{
    class MetaConfig; 
};

namespace iLogW
{

    // Настройки вывода трассировки
    class LogSessionSettings
    {
    public:
        LogSessionSettings() :
          m_isTimestampUsed(false),
          m_isOutputToCoutOn(false),
          m_timeoutSyncroToStore(-1),
          m_countSyncroToStore(-1) 
          {}

        LogSessionSettings(Utils::MetaConfig &); 

        bool isTimestampUsed() const {  return m_isTimestampUsed; }
        void setTimestampInd(bool par) { m_isTimestampUsed = par; }

        bool isOutputToCoutOn() const {  return m_isOutputToCoutOn; }
        void setOutputToCout(bool useCout) {  m_isOutputToCoutOn = useCout; }

        bool isOutputToFileOn() const {  return m_logFileName.size() != 0; }
        const std::string &getLogFileName() const { return m_logFileName; }
        void setLogFileName(const std::string &name) { m_logFileName = name; }

        int  getTimeoutSyncroToStore() const {  return m_timeoutSyncroToStore; }
        void setTimeoutSyncroToStore(int  timeout) {  m_timeoutSyncroToStore = timeout; }

        int  getCountSyncroToStore() const {    return m_countSyncroToStore; }
        void setCountSyncroToStore(int  count) {    m_countSyncroToStore = count; }
    private:
        bool m_isTimestampUsed;     // Timestamp is used
        bool m_isOutputToCoutOn;    // Is output to console turn on
        std::string m_logFileName;  // Name of log file. If name present, than record write to file immediate
        int  m_timeoutSyncroToStore;// Timeout for synchronization with store
        int  m_countSyncroToStore;  // Count of records for synchronization
    };
}; 

#endif 


