#include "stdafx.h"
#include "Utils/MetaConfig.h"
#include "LogSessionSettings.h"

namespace iLogW
{
    LogSessionSettings::LogSessionSettings(Utils::MetaConfig &config)
    {
        config.Add(m_isTimestampUsed, "Timestamp is used");
        config.Add(m_isOutputToCoutOn, "Output to console");
        config.Add(m_logFileName, "Name of log file");
        config.Add(m_timeoutSyncroToStore, "Timeout for synchronization with log store");
        config.Add(m_countSyncroToStore,   "Count of records for synchronization");
    }
}; 


