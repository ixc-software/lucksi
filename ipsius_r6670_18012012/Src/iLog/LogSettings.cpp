#include "stdafx.h"
#include "Utils/MetaConfig.h"
#include "LogSettings.h"

namespace iLogW
{
    void LogSettings::RegisterInMetaConfig(Utils::MetaConfig &config)
    {
        config.Add(m_isTimestampUsed, "Timestamp", "true if timestamp is used");
        config.Add(m_timeoutSyncroToStore, "synchroTimeout","Timeout for synchronization with log store");
        config.Add(m_countSyncroToStore, "countSynchroRecord",  "Count of records for synchronization with log store");
        m_out.RegisterInMetaConfig(config);
    }
}; 


