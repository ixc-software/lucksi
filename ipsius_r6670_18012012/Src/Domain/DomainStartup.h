
#ifndef __DOMAINSTARTUP__
#define __DOMAINSTARTUP__

// DomainStartup.h

#include "Utils/IBasicInterface.h"
#include "Utils/HostInf.h"
#include "Utils/StringList.h"
#include "iLog/LogSettings.h"
#include "DRI/DriIncludeDirList.h"
#include "iCore/ThreadRunner.h"

#include "ObjectName.h"
#include "IDomainInnerCreate.h"

namespace Domain
{
    enum DomainExitCode
    {
        DomainExitOk = 0,
        DomainExitErr = -1,
        DomainExitEsc = -2,
    };

    // -------------------------------------------------
    
    // Domain startup params.
    // Default parameter IncludeDirList can be invalid, so call Verify() before using.
    // It throws exception if any error occures durring commandLine parsing.
    class DomainStartup : public iCore::IThreadRunnerEvents
    {
        typedef DomainStartup T;

        // From command line:
        // for domain
        DomainName m_domainName;
        bool m_suppressVerbose;
        bool m_disableExitByKey;
        bool m_exitAfterAutoRun;
        QString m_password;
        // for telnet
        Utils::HostInf m_telnetDriAddr;
        // for scripts
        DRI::DriIncludeDirList m_dirs;
        // for autorun
        QString m_scriptFile; // check existance via m_dirs
        bool m_enableScriptTrace;
        bool m_disableExitOnAutorunError;
        // for log
        iLogW::LogSettings m_logSettings;
        // Utils::HostInf m_remoteLogAddr; // in logSettings
        int m_logCmdResultTrimVal;
        // special
        QStringList m_specialParams;
        
        // help
        QString m_help;     // if help's requested than it has list of options
        QString m_helpKeys; // using in error msgs
        
        // other
        bool m_isMOD;
        IDomainInnerCreate *m_pInnerCreate;
        int m_timeout;
        DomainExitCode m_exitCode;
        iCore::IThreadRunnerEvents *m_pThreadEvents;
        std::string m_buildInfo;
        
        void Error(const QString &desc, const QString &src);

        void Init();
        void InitLogSettings();
        
        // void TrySaveAddress(const QString &src, Utils::HostInf &res);
        void TrySaveDomainName(const std::string &name);
        void TrySaveDirs(const std::string &src);
        void CheckIfScriptFileExists();
        void Parse(const Utils::StringList &params);

    // iCore::IThreadRunnerEvents
    private:

        void OnCreateObjectFail(iCore::IThreadRunner &sender) 
        {
            //if (m_pThreadEvents) m_pThreadEvents->OnCreateObjectFail(sender);
        }

        void OnTimeout(iCore::IThreadRunner &sender) 
        {
            //if (m_pThreadEvents) m_pThreadEvents->OnTimeout(sender);
        }

        void OnThreadUnexpectedStop(iCore::IThreadRunner &sender) 
        {
            //if (m_pThreadEvents) m_pThreadEvents->OnThreadUnexpectedStop(sender);
        }

        // this called from MsgThread context
        void OnThreadException(iCore::IThreadRunner &sender, 
            const std::exception *pE, bool *pSuppressIt) 
        {
            if (m_pThreadEvents) m_pThreadEvents->OnThreadException(sender, pE, pSuppressIt);
        }

        // this called from pool thread context
        void OnThreadPoolException(iCore::IThreadRunner &sender, 
            const std::exception *pE, bool *pSuppressIt) 
        {
            if (m_pThreadEvents) m_pThreadEvents->OnThreadPoolException(sender, pE, pSuppressIt);
        }
        
    public:
        ESS_TYPEDEF(DomainStartupException);

        DomainStartup();

        // can throw
        DomainStartup(const Utils::StringList &commandLine, const std::string &buildInfo = ""); 

        // for DomainTestRunner
        DomainStartup(const iLogW::LogSettings &sett); 

        bool HelpRequested() const { return !m_help.isEmpty(); }
        const QString& Help() const { return m_help; }

        const DomainName& Name() const { return m_domainName; }
        void Name(const DomainName &val) { m_domainName = val; }
        
        const Utils::HostInf& TelnetDriAddr() const { return m_telnetDriAddr; }
        void TelnetDriAddr(const Utils::HostInf &val) { m_telnetDriAddr = val; }

        const DRI::DriIncludeDirList& IncludeDirList() const;
        void IncludeDirList(const DRI::DriIncludeDirList &val);
        
        bool SuppressVerbose() const { return m_suppressVerbose; }
        void SuppressVerbose(bool val) { m_suppressVerbose = val; }

        const QString& ScriptFileName() const { return m_scriptFile; } 
        void ScriptFileName(const QString &val) { m_scriptFile = val; } 

        bool EnableScriptTrace() const { return m_enableScriptTrace; }
        void EnableScriptTrace(bool val) { m_enableScriptTrace = val; }

        bool DisableExitByKey() const { return m_disableExitByKey; }
        void DisableExitByKey(bool val) { m_disableExitByKey = val; }

        const Utils::HostInf& RemoteLogAddr() const;
        void RemoteLogAddr(const Utils::HostInf &val);

        const QString& Password() const { return m_password; }
        void Password(const QString &val) { m_password = val; }

        bool DisableExitOnAutorunError() const { return m_disableExitOnAutorunError; }
        void DisableExitOnAutorunError(bool val) { m_disableExitOnAutorunError = val; }

        bool ExitAfterAutoRun() const { return m_exitAfterAutoRun; }
        void ExitAfterAutoRun(bool val) { m_exitAfterAutoRun = val; }

        int LogCmdResultTrim() const { return m_logCmdResultTrimVal; }
        void LogCmdResultTrim(int val) { m_logCmdResultTrimVal = val; }

        // Starts from 1
        const QStringList& SpecialParams() const { return m_specialParams; }
        void SpecialParams(const QStringList &params);
       
        const iLogW::LogSettings& LogSettings() const { return m_logSettings; }

        bool IsMOD() { return m_isMOD; }
        void IsMOD(bool isMOD) { m_isMOD = isMOD; }

        IDomainInnerCreate* DomainInnerCreate() { return m_pInnerCreate; }
        void DomainInnerCreate(IDomainInnerCreate *pValue) { m_pInnerCreate = pValue; }

        int Timeout() const { return m_timeout; }
        void Timeout(int value) { m_timeout = value; }

        // bool Authorize(const QString &userName, const QString &password) const;

        DomainExitCode ExitCode() const { return m_exitCode; }
        void ExitCode(DomainExitCode val);

        const std::string& BuildInfo() const { return m_buildInfo; }

        void SetThreadEventsHandler(iCore::IThreadRunnerEvents &handler)
        {
            ESS_ASSERT(m_pThreadEvents == 0);
            m_pThreadEvents = &handler;
        }

        void ClearThreadEventsHandler()
        {
            m_pThreadEvents = 0;
        }

    };
    
} // namespace Domain

#endif

