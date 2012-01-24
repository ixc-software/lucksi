#include "stdafx.h"

#include "Utils/ExeName.h"
#include "Utils/StringList.h"

#include "MsgThreadMonitor.h"

// -------------------------------------------------------------

namespace iCore
{
    
    void MsgThreadMonitor::OnMonitoringError(iCore::MsgThread &t)
    {
        try
        {
            std::string queueInfo;
            t.GetMsgQueueDebugInfo(queueInfo, 32);

            std::string profilingInfo;
            iCore::IMsgProcessorProfiler *p = t.MsgProfiler();
            if (p) p->GetDebugInfo(profilingInfo);

            std::string info;
            info += queueInfo;
            if (!profilingInfo.empty())
            {
                info += "\n\n";
                info += profilingInfo;
            }

            QString name( Utils::ExeName::GetExeDir().c_str() );
            name += "ThreadMonitorCrushInfo.txt";

            Utils::StringList sl;
            sl.push_back( info.c_str() );
            sl.SaveToFile(name);
        }
        catch(...)
        {
            // yes, ignore anything
        }
    }

    
}  // namespace iCore

