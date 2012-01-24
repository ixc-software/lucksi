
#include "stdafx.h"

#include "DomainStartup.h"
#include "Identificator.h"
#include "DomainStartupConsts.h"

#include "Utils/QtHelpers.h"
#include "Utils/ProgramOptions.h"
#include "Utils/ProgramOptionsSetters.h"

namespace 
{
    std::vector<std::string> Flags(const std::string &first, 
                                   const std::string &second = "", 
                                   const std::string &third = "")
    {
        std::vector<std::string> res;

        ESS_ASSERT(!first.empty());
        res.push_back(first);
        if (!second.empty()) res.push_back(second);
        if (!third.empty()) res.push_back(third);

        return res;
    }
    
} // namespace 

// ---------------------------------------------------------

namespace Domain
{
    // ---------------------------------------------------------
    // DomainStartup impl

    DomainStartup::DomainStartup() : 
        m_domainName(DomainStartupDetails::Defaults::DomainName())
    {
        Init();
    }

    // ---------------------------------------------------------
    
    DomainStartup::DomainStartup(const Utils::StringList &commandLine, 
        const std::string &buildInfo) :  
        m_domainName(DomainStartupDetails::Defaults::DomainName()),
        m_buildInfo(buildInfo)
    {
        Init();

        if (commandLine.isEmpty()) return;
        
        Parse(commandLine);
        CheckIfScriptFileExists();
    }

    // ---------------------------------------------------------

    DomainStartup::DomainStartup(const iLogW::LogSettings &sett) :
        m_domainName(DomainStartupDetails::Defaults::DomainName())
    {
        Init();
        
        m_logSettings = sett;
    }

    // ---------------------------------------------------------

    void DomainStartup::Init()
    {
        using namespace DomainStartupDetails;

        DomainName(DomainName(Defaults::DomainName()));
        TelnetDriAddr(Utils::HostInf(Defaults::TelnetDriAddr(), Defaults::TelnetDriPort()));
        IncludeDirList(DRI::DriIncludeDirList(Defaults::IncludeDirs()));
        EnableScriptTrace(Defaults::EnableScriptTrace());
        ScriptFileName(Defaults::AutoRunScriptFile()); 
        RemoteLogAddr(Utils::HostInf(Defaults::RemoteLogAddr(), Defaults::RemoteLogPort()));
        SuppressVerbose(Defaults::SuppressVerbose());
        DisableExitByKey(Defaults::DisableExitByKey());
        Password(Defaults::Password());
        DisableExitOnAutorunError(Defaults::DisableExitOnAutorunError());
        LogCmdResultTrim(Defaults::LogCmdResultTrim());
        ExitAfterAutoRun(Defaults::ExitAfterAutoRun());

        IsMOD(false);
        DomainInnerCreate(0);
        Timeout(0);
        ExitCode(DomainExitOk);
        m_pThreadEvents = 0;
        
        InitLogSettings();
    }

    // ---------------------------------------------------------

    void DomainStartup::InitLogSettings()
    {
        // m_logSettings.SetStoreRecordsLimit(1024, 512);
        // m_logSettings.TimestampInd(true);
        m_logSettings.CountSyncroToStore(1);  // sync on every records
        m_logSettings.UseRecordsReorder(true);
        m_logSettings.DublicateSessionNamesDetection(true);
    }

    // ---------------------------------------------------------

    void DomainStartup::Error(const QString &desc, const QString &src)
    {
        ESS_ASSERT(!m_helpKeys.isEmpty());
        std::string msg = QString("Domain startup error: %1; source - %2. "
                                  "Type %3 for help.").arg(desc).arg(src)
                                  .arg(m_helpKeys).toStdString();
        ESS_THROW_MSG(DomainStartupException, msg);
    }

    // ---------------------------------------------------------
    
    void DomainStartup::TrySaveDirs(const std::string &src)
    {
        if (!src.empty()) m_dirs.Clear();

        QString s = QString::fromStdString(src);
        m_dirs.Add(s);

        QStringList invalidDirs;
        if (m_dirs.Verify(&invalidDirs)) return;
        
        Error("Invalid include dirrectories", invalidDirs.join("; "));
    }
    
    // ---------------------------------------------------------

    void DomainStartup::CheckIfScriptFileExists()
    {
        if (m_scriptFile.isEmpty()) return;

        try
        {
            m_scriptFile = m_dirs.Find(m_scriptFile);
        }
        catch(DRI::DriIncludeDirList::InvalidPath &e)
        {
            Error("Invalid script file", e.getTextMessage().c_str());
        }
    }
    
    // ---------------------------------------------------------

    void DomainStartup::TrySaveDomainName(const std::string &name)
    {
        QString n = QString::fromStdString(name);
        try
        {
            m_domainName = DomainName(n);
        }
        catch(ObjectNameError &e)
        {
            Error("Invalid domain name", n);
        }
    }

    // ---------------------------------------------------------

    void DomainStartup::Parse(const Utils::StringList &params)
    {
        typedef DomainStartupDetails::Flags F;
        using namespace Utils;
        
        Utils::ProgramOptions<DomainStartup> opt(*this);
        
        opt.Add(&T::TrySaveDomainName, 
                Flags(F::DomainName(), F::DomainName2()),
                "domain name");

        opt.Add(&T::TrySaveDirs, 
                Flags(F::IncludeDirs(), F::IncludeDirs2()), 
                "include dirrectories list, format: \"path1; path2\"");

        opt.Add(SetOptHostInf(m_telnetDriAddr,  
                Flags(F::TelnetDriAddr(), F::TelnetDriAddr2(), F::TelnetDriAddr3()),
                "domain address, format: host:port"));

        Utils::HostInf logAddr;
        opt.Add(SetOptHostInf(logAddr,
                Flags(F::RemoteLogAddr(), F::RemoteLogAddr2(), F::RemoteLogAddr3()),
                "address for log capturer, format: host:port(UDP)"));

        opt.Add(SetOptQString(m_scriptFile,     
                Flags(F::AutoRunScriptFile(), F::AutoRunScriptFile2(), F::AutoRunScriptFile3()),
                "domain start script file name"));
        
        opt.Add(SetOptBool(m_enableScriptTrace, 
                Flags(F::EnableScriptTrace(), F::EnableScriptTrace2(), F::EnableScriptTrace3()),
                "enable displaying autorun info"));
        
        opt.Add(SetOptBool(m_exitAfterAutoRun,  
                Flags(F::ExitAfterAutoRun(), F::ExitAfterAutoRun2(), F::ExitAfterAutoRun3()),
                "exit after autorun scrip was completed"));

        opt.Add(SetOptBool(m_suppressVerbose,   
                Flags(F::SuppressVerbose(), F::SuppressVerbose2(), F::SuppressVerbose3()),
                "disable verbose output"));
        
        opt.Add(SetOptBool(m_disableExitByKey,  
                Flags(F::DisableExitByKey(), F::DisableExitByKey2(), F::DisableExitByKey3()),
                "disable exit by key"));
        
        opt.Add(SetOptQString(m_password,
                Flags(F::Password(), F::Password2()),
                "password"));
        
        opt.Add(SetOptBool(m_disableExitOnAutorunError,  
                Flags(F::DisableExitOnAutorunError(), F::DisableExitOnAutorunError2(),
                      F::DisableExitOnAutorunError3()),
                "disable exit after error in autorun scrip"));
        
        opt.Add(SetOptInt(m_logCmdResultTrimVal,
                Flags(F::LogCmdResultTrim(), F::LogCmdResultTrim2(), F::LogCmdResultTrim3()),
                "trim command result to given character number (0 - disable)"));
        
        opt.Add(SetOptStringList(m_specialParams, 
                Flags(F::SpecialParams(), F::SpecialParams2(), F::SpecialParams3()),
                "special params that can be used from within session, they can be "
                "accessed as %_1%, %_2%, etc. %_0% - .exe file name by default"));
        
        opt.AddDefaultHelpOption();
        
        m_helpKeys = QString::fromStdString(opt.HelpKeys());
        
        try
        {
            opt.Process(params);
        }
        catch (ProgramOptionsExceptions::Base &e)
        {
            Error(QString::fromStdString(e.getTextMessage()), "command line processing");
        }

        if (!logAddr.Empty())
        {
            if ( logAddr.IsValid(false) )
            {
                if (logAddr.Address() == "0.0.0.0") logAddr.Address("127.0.0.1");
                RemoteLogAddr(logAddr);
            }
            else
            {
                Error("Bad log address!", "");
            }
        }

        // save help info
        if (opt.HelpRequested()) 
        {
            m_help = QString::fromStdString(opt.HelpInfo());
        }

    }

    // ---------------------------------------------------------

    void DomainStartup::IncludeDirList(const DRI::DriIncludeDirList &val)
    {
        m_dirs = val;
    }

    // ---------------------------------------------------------
    
    const DRI::DriIncludeDirList& DomainStartup::IncludeDirList() const
    {
        // ESS_ASSERT(m_dirs.Verify());

        return m_dirs;
    }

    // ---------------------------------------------------------

    /*
    bool DomainStartup::Authorize(const QString &userName, const QString &password) const
    {
        if (m_password.isEmpty()) return true;
        
        return (password == m_password);
    } */

    // ---------------------------------------------------------

    /*int DomainStartup::SpecialParamCount() const
    {
        return m_specialParams.size();
    }

    // ---------------------------------------------------------

    const QString& DomainStartup::SpecialParam(int number) const
    {
        // if (numer >= m_specialParams.size())
        // {
        //     Error("Request unknown special parameter", QString::number(number));
        // }
        ESS_ASSERT(number > 0);
        ESS_ASSERT(number <= SpecialParamCount());

        return m_specialParams.at(number - 1);
    }*/

    // ---------------------------------------------------------
    
    void DomainStartup::SpecialParams(const QStringList &params)
    {
        m_specialParams = params;
    }

    // ---------------------------------------------------------

    const Utils::HostInf& DomainStartup::RemoteLogAddr() const
    {
        return m_logSettings.out().Udp().DstHost;
    }

    // ---------------------------------------------------------
    
    void DomainStartup::RemoteLogAddr(const Utils::HostInf &val)
    {
        m_logSettings.out().Udp().DstHost = val;
        m_logSettings.out().Udp().TraceInd = true;
    }

    // ---------------------------------------------------------

    void DomainStartup::ExitCode(DomainExitCode val)
    {
        m_exitCode = val;
    }

    // ---------------------------------------------------------
    
} // namespace Domain
