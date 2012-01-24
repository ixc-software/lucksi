#ifndef __LOGSTAT__
#define __LOGSTAT__

#include "Platform/PlatformTypes.h"

namespace iLog
{
    using Platform::dword;
    using Platform::ddword; 
    
    struct LogStat
    {
        // current
        dword  CurrRecords;
        dword  CurrSessions;

        // total
        dword  TotalRecords;       // in
        ddword TotalRecordsData;   // in
        dword  TotalRecordsStored; // in list, <= TotalRecords
        dword  TotalSessions;

        LogStat()
        {
            CurrRecords = 0;
            CurrSessions = 0;

            TotalRecords = 0;
            TotalRecordsData = 0;            
            TotalRecordsStored = 0;
            TotalSessions = 0;
        }

        std::string ToString(const std::string &sep = "\n") const
        {
            std::ostringstream oss;
            oss << "CurrRecords "           << CurrRecords << sep
                << "CurrSessions "          << CurrSessions << sep
                << "TotalRecords "          << TotalRecords << sep
                << "TotalRecordsData "      << TotalRecordsData << sep
                << "TotalRecordsStored "    << TotalRecordsStored << sep
                << "TotalSessions "         << TotalSessions << sep;

            return oss.str();
        }

    };
    
    
}  // namespace iLog

#endif
