
#include "stdafx.h"

#include "DomainTests.h"
#include "Domain/DomainStartupConsts.h"
#include "Domain/DomainStartup.h"
#include "Utils/QtHelpers.h"

namespace 
{
    using namespace Domain;
    using namespace Domain::DomainStartupDetails;
    using namespace DRI;
   
    struct FieldList
    {
        Utils::HostInf TelnetDriAddr;
        Utils::HostInf RemoteLogAddr;
        QString Name;
        DriIncludeDirList Dirs;
        QString AutoRunScriptFile;
        bool SuppressVerbose;
        bool EnableScriptTrace;
        bool DisableExitByKey;
        QString Password;

        FieldList() : 
            TelnetDriAddr(Defaults::TelnetDriAddr(),Defaults::TelnetDriPort()), 
            RemoteLogAddr(Defaults::RemoteLogAddr(),Defaults::RemoteLogPort()), 
            Name(Defaults::DomainName()), 
            Dirs(Defaults::IncludeDirs()), 
            AutoRunScriptFile(Defaults::AutoRunScriptFile()), 
            SuppressVerbose(Defaults::SuppressVerbose()), 
            EnableScriptTrace(Defaults::EnableScriptTrace()),
            DisableExitByKey(Defaults::DisableExitByKey()),
            Password(Defaults::Password())
        {
        }

        bool IsSame(const DomainStartup &config, bool silentMode) const
        {
            if(!silentMode)
            {
                std::cout << "IsSame:" << std::endl
                     << TelnetDriAddr.ToString() << "==" << config.TelnetDriAddr().ToString() << std::endl
                     << RemoteLogAddr.ToString() << "==" << config.RemoteLogAddr().ToString() << std::endl
                     << Name << "==" << config.Name().Name() << std::endl
                     << Dirs.ToString() << "==" << std::endl
                     << config.IncludeDirList().ToString() << std::endl
                     << AutoRunScriptFile << "==" << config.ScriptFileName() << std::endl
                     << SuppressVerbose << "==" << config.SuppressVerbose() << std::endl
                     << EnableScriptTrace << "==" << config.EnableScriptTrace() << std::endl
                     << DisableExitByKey << "==" << config.DisableExitByKey() << std::endl
                     << Password << "==" << config.Password() << std::endl
                     << std::endl;
            }
            
            return ((TelnetDriAddr == config.TelnetDriAddr())
                     && (RemoteLogAddr == config.RemoteLogAddr())
                     && (Name == config.Name().Name())
                     && (Dirs == config.IncludeDirList())
                     && (config.ScriptFileName().contains(AutoRunScriptFile, Qt::CaseSensitive))
                     && (SuppressVerbose == config.SuppressVerbose())
                     && (EnableScriptTrace == config.EnableScriptTrace())
                     && (DisableExitByKey == config.DisableExitByKey())
                     && (Password == config.Password()));
        }
    };

    // ------------------------------------------------------

    // command line
    Utils::StringList cl(const QString &s1, const QString &s2 = "", const QString &s3 = "")
    {
        Utils::StringList res;
        res << "exeName" << s1 << s2 << s3;
        return res;
    }

    // ------------------------------------------------------

    // short flag 
    QString shf(const QString &name)
    {
        return QString("-").append(name);
    }
    
    // ------------------------------------------------------

    Utils::HostInf HostInfFromStr(const QString &str)
    {
        if (str.isEmpty()) return Utils::HostInf(0);
        
        QStringList sl = str.split(Consts::AddrSep());
        ESS_ASSERT(sl.size() == 2);

        bool portOK = false;
        int port = sl.at(1).toInt(&portOK);
        ESS_ASSERT(portOK);
        
        return Utils::HostInf(sl.at(0).toStdString(), port);
    }
    
    // ------------------------------------------------------
    
    void Check(const Utils::StringList &cmdLine, const FieldList &toCompare, bool silentMode)
    {
        if (!silentMode) 
        {
            std::cout << "Command line: |" << cmdLine.join("|") << std::endl;
        }
        DomainStartup config(cmdLine);
        TUT_ASSERT(toCompare.IsSame(config, silentMode));
    }

    // ------------------------------------------------------

    void CheckInvalid(const Utils::StringList &cmdLine, bool silentMode)
    {
        try
        {
            DomainStartup config(cmdLine);
        }
        catch (DomainStartup::DomainStartupException &e)
        {
            if (!silentMode) std::cout << e.getTextMessage() << std::endl;
            return;
        }
        
        TUT_ASSERT(0 && "No Exception!");
    }

    // ------------------------------------------------------

    void MakeCheckPair(const QString &telnetDriAddr, const QString &remoteLogAddr, 
                       const QString &name, const QString &dirs, 
                       const QString &scriptFile, bool suppressVerbose, 
                       bool enableScriptTrace, bool disableExitByKey,
                       const QString &password,
                       Utils::StringList &cmdLine, FieldList &fl)
    {
        cmdLine.clear();
        cmdLine << "exe name";
        if (!telnetDriAddr.isEmpty()) 
        {
            cmdLine << shf(Flags::TelnetDriAddr()) << telnetDriAddr;
            fl.TelnetDriAddr = HostInfFromStr(telnetDriAddr);
        }
        if (!remoteLogAddr.isEmpty())
        {
            cmdLine << shf(Flags::RemoteLogAddr()) << remoteLogAddr;
            fl.RemoteLogAddr = HostInfFromStr(remoteLogAddr);
        }
        if (!name.isEmpty())
        {
            cmdLine << shf(Flags::DomainName()) << name;
            fl.Name = name;
        }
        if (!dirs.isEmpty()) 
        {
            cmdLine << shf(Flags::IncludeDirs()) << dirs;
            fl.Dirs = DRI::DriIncludeDirList(dirs);
        }
        if (!scriptFile.isEmpty()) 
        {
            cmdLine << shf(Flags::AutoRunScriptFile()) << scriptFile; 
            fl.AutoRunScriptFile = scriptFile;
        }
        if (suppressVerbose)
        {
            cmdLine << shf(Flags::SuppressVerbose()); 
            fl.SuppressVerbose = suppressVerbose;
        }
        if (enableScriptTrace)
        {
            cmdLine << shf(Flags::EnableScriptTrace()); 
            fl.EnableScriptTrace = enableScriptTrace;
        }
        if (disableExitByKey)
        {
            cmdLine << shf(Flags::DisableExitByKey()); 
            fl.DisableExitByKey = disableExitByKey;
        }
        if (!password.isEmpty()) 
        {
            cmdLine << shf(Flags::Password()) << password; 
            fl.Password = password;
        }
    }
    
    // ------------------------------------------------------

    void TestValid(bool silentMode)
    {
        QChar sep = DRI::DriIncludeDirList::PathSeparator();
        
        QString telnetDriAddr("156.13.14.12:5689");
        QString remoteAddr("168.75.45.0:9668");
        QString name("Domain1");
        QString dirs = QString(" ../isc/").append(sep).append("../").append(sep).append("./");
        QString scriptFile("TestExtern.isc");
        QString password = "2login";
        
        // Check(cl(""), FieldList(), silentMode);
        
        // check in diff order
        {
            Utils::StringList sl;
            FieldList fl;
            MakeCheckPair(telnetDriAddr, remoteAddr, name, dirs, scriptFile, true, true, false, password,  sl, fl);
            Check(sl, fl, silentMode);
        }
        {
            Utils::StringList sl;
            FieldList fl;
            MakeCheckPair(telnetDriAddr, "", name, dirs, "", false, true, false, "",  sl, fl);
            Check(sl, fl, silentMode);
        }
        {
            Utils::StringList sl;
            FieldList fl;
            MakeCheckPair(telnetDriAddr, "", "", "", "", true, false, false, password, sl, fl);
            Check(sl, fl, silentMode);
        }
        {
            Utils::StringList sl;
            FieldList fl;
            MakeCheckPair("", "", name, dirs, "", false, false, false, "", sl, fl);
            Check(sl, fl, silentMode);
        }
    }

    // ------------------------------------------------------

    void TestInvalid(bool silentMode)
    {
        // invalid param
        CheckInvalid(cl("a"), silentMode);
        CheckInvalid(cl("-x"), silentMode);
        CheckInvalid(cl(shf(Flags::DomainName()), "name1", "name2"), silentMode);
        CheckInvalid(cl(shf(Flags::IncludeDirs()), "../", "name2"), silentMode);
        CheckInvalid(cl(shf(Flags::IncludeDirs()), "../", "./"), silentMode);
        
        // invalid address
        CheckInvalid(cl(shf(Flags::TelnetDriAddr()), "_invalid_adress_"), silentMode);
        CheckInvalid(cl(shf(Flags::TelnetDriAddr()), "123.22.36.69:_invalid_port_"), silentMode);
        CheckInvalid(cl(shf(Flags::TelnetDriAddr()), "123.800.36.69:4589"), silentMode);
        CheckInvalid(cl(shf(Flags::TelnetDriAddr()), Flags::DomainName()), silentMode);

        // invalid log address
        CheckInvalid(cl(shf(Flags::RemoteLogAddr()), "_invalid_adress_"), silentMode);
        CheckInvalid(cl(shf(Flags::RemoteLogAddr()), "123.22.36.69:_invalid_port_"), silentMode);
        CheckInvalid(cl(shf(Flags::RemoteLogAddr()), "123.800.36.69:4589"), silentMode);
        CheckInvalid(cl(shf(Flags::RemoteLogAddr()), Flags::TelnetDriAddr()), silentMode);

        // invalide domain name
        CheckInvalid(cl(shf(Flags::DomainName()), "1Domain"), silentMode);
        CheckInvalid(cl(shf(Flags::DomainName()), "Dom%ain"), silentMode);

        // invalid dirs
        CheckInvalid(cl(shf(Flags::IncludeDirs()), "_invalidDir_"), silentMode);
        CheckInvalid(cl(shf(Flags::IncludeDirs()), "_invalidDir_/"), silentMode);
        CheckInvalid(cl(shf(Flags::IncludeDirs()), "\"_invalidDir_/ _invalidDir2_/ \""), silentMode);

        // non-exist script file
        CheckInvalid(cl(shf(Flags::AutoRunScriptFile()), "_invalid_script_file_.isc"), silentMode);
    }
    
    // ------------------------------------------------------
    
} // namespace

// ------------------------------------------------------

namespace DomainTests
{
    void DomainStartupTest(bool silentMode)
    {
        TestValid(silentMode);

        TestInvalid(silentMode);
        
        if (!silentMode) std::cout << "DomainStartupTest: OK" << std::endl;
    }
    
} // namespace DomainTests
