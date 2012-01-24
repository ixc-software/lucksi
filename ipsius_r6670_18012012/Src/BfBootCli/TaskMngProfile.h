#ifndef TASKMNGPROFILE_H
#define TASKMNGPROFILE_H

#include "iCore/MsgThread.h"
#include "ITaskManagerReport.h"
#include "iLog/LogWrapper.h"
#include "Domain/DomainClass.h"

namespace BfBootCli
{
    struct TaskMngProfile
    {        
        Domain::IDomain &Domain;
        ITaskManagerReport& Owner;
        bool TraceActive;
        Utils::SafeRef<iLogW::ILogSessionCreator> LogCreator;

        TaskMngProfile( Domain::IDomain &domain, ITaskManagerReport &owner, 
            bool traceActive, Utils::SafeRef<iLogW::ILogSessionCreator> logCreator) :             
            Domain(domain),
            Owner(owner),
            TraceActive(traceActive),
            LogCreator(logCreator)
        {
        }        
    };
  
} // namespace BfBootCli

#endif
