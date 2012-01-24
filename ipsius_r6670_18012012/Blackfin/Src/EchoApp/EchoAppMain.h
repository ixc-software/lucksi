#ifndef __ECHOAPPMAIN__
#define __ECHOAPPMAIN__

#include "iLog/LogSettings.h"

namespace EchoApp
{
    
    struct EchoAppProfile
    {
        bool MonitorCpuUsage;
        int ThreadRunnerSleepInterval;

        std::string AppName;
        std::string BuildInfo;

        // log    
        iLogW::LogSettings LogConfig;
        bool LogTrace;

        EchoAppProfile()
        {
            MonitorCpuUsage = true;
            ThreadRunnerSleepInterval = 10 * 1000;

            AppName = "EchoApp";
            BuildInfo = "(no build info)";

            // LogSettings
            {
                LogConfig.StoreRecordsLimit(0);
                LogConfig.StoreCleanTo(0);
                LogConfig.TimestampInd(true);
                LogConfig.CountSyncroToStore(1);  // sync on every records
                LogConfig.UseRecordsReorder(false);
                LogConfig.DublicateSessionNamesDetection(false);

                LogTrace = false;
            }

        }

    };
    
    void RunEchoApp(const EchoAppProfile &profile);
    
    
}  // namespace EchoApp

#endif
