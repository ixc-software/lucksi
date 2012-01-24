
#include "stdafx.h"

#include "DomainTestRunner.h"
#include "Utils/StringList.h"

namespace Domain
{
    DomainTestRunner::DomainTestRunner(int timeout)
    {
        SetupParams(timeout);
    }

    // -----------------------------------------------------

    DomainTestRunner::DomainTestRunner(const iLogW::LogSettings &sett, int timeout) :
        m_startup(sett)
    {
        SetupParams(timeout);
    }

    // -----------------------------------------------------

    DomainTestRunner::DomainTestRunner(const QStringList &specialParams, int timeout)
    {
        SetupParams(timeout);
        m_startup.SpecialParams(specialParams);
    }

    // -----------------------------------------------------
    
    void DomainTestRunner::SetupParams(int timeout)
    {
        m_startup.Name(DomainName("TestDomain"));
        m_startup.TelnetDriAddr(Utils::HostInf("127.0.0.1", 0));
        if (!m_startup.RemoteLogAddr().IsValid())
        {
            m_startup.RemoteLogAddr(Utils::HostInf("127.0.0.1", 56001));
        }
        m_startup.SuppressVerbose(true);
        m_startup.Timeout(timeout);
        m_startup.Password("test");
    }

    // -----------------------------------------------------
    
    bool DomainTestRunner::TryRun()
    {
        DomainRunner runner;

        return runner.Run(m_startup);
    }

    // -----------------------------------------------------

} // namespace Domain
