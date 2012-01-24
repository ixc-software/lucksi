
#include "stdafx.h"

#include "DomainRunner.h"
#include "DomainStartup.h"
#include "DomainClass.h"
#include "iCore/ThreadRunner.h"

// -------------------------------------------------------

namespace
{
    using namespace iCore;
    using namespace Domain;

    //class Events : public iCore::IThreadRunnerEvents
    //{
    //    DomainStartup &m_params;

    //// IThreadRunnerEvents impl
    //private:

    //    void OnCreateObjectFail(IThreadRunner &sender) {}
    //    void OnTimeout(IThreadRunner &sender) {}
    //    void OnThreadUnexpectedStop(IThreadRunner &sender) {}

    //    // this called from another thread context
    //    void OnThreadException(IThreadRunner &sender, 
    //        const std::exception *pE, bool *pSuppressIt) {}
    //    void OnThreadPoolException(IThreadRunner &sender, 
    //        const std::exception *pE, bool *pSuppressIt) {}

    //public:

    //    Events(DomainStartup &params) : m_params(params)
    //    {
    //    }
    //};

}  // namespace

// -------------------------------------------------------

namespace Domain
{

    bool DomainRunner::Run(DomainStartup &params)
    {        
        iCore::ThreadRunner runner(Platform::Thread::LowPriority, params.Timeout());

        std::string threadName = "DomainClass";
        std::string domainName = params.Name().Name().toStdString();
        if (!domainName.empty()) threadName += ":" + domainName;

        return runner.Run<DomainClass>(threadName, params, &params);
    }

}  // namespace Domain
