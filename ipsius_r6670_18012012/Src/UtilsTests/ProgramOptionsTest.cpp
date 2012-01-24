

#include "stdafx.h"
#include "ProgramOptionsTest.h"
#include "Utils/ProgramOptions.h"
#include "Utils/ProgramOptionsSetters.h"
#include "Utils/StringList.h"
#include "Utils/QtHelpers.h"

namespace 
{
    using namespace Utils;

    /*
    SetOptString
    SetOptQString
    SetOptInt
    SetOptHostInf
    SetOptBool

    DefaultHelp

    Fn()
    Fn(string)
    Fn(vector<string>)
    */

    struct ExecOpts
    {
        std::string ExecStr;
        std::vector<std::string> ExecList;
        bool ExecNoParam;
        
        ExecOpts() : ExecNoParam(false)
        {
        }
       
        bool IsSame(const ExecOpts &other) const
        {
            bool res = (ExecNoParam == other.ExecNoParam) && (ExecStr == other.ExecStr);

            if (ExecList.size() != other.ExecList.size()) return false;
            for (size_t i = 0; i < other.ExecList.size(); ++i)
            {
                if (ExecList.at(i) != other.ExecList.at(i)) return false;
            }

            return res;
        }

        void Validate()
        {
            ESS_ASSERT(!ExecStr.empty());
            ESS_ASSERT(!ExecList.empty());
            ESS_ASSERT(ExecNoParam == true);
        }
    };

    // ---------------------------------------------------

    struct ConfigData
    {
        // using set-mechanism
        std::string OptStr;
        QString OptQStr;
        int OptInt;
        Utils::HostInf OptAddr;
        bool OptEnable;
        bool OptSet;

        // using call-function mechanism
        ExecOpts OptExec;

        // using to test default help
        bool OptHelp;

        ConfigData() : OptInt(-1), OptEnable(false), OptSet(false), OptHelp(false)
        {
        }

        bool IsSame(const ConfigData &other, bool silentMode) const 
        {
            if (!silentMode) 
            {
                std::cout << "OptStr -- " << OptStr << " == " << other.OptStr << std::endl
                    << "OptQStr -- " << OptQStr.toStdString() << " == " 
                                     << other.OptQStr.toStdString() << std::endl
                    << "OptInt -- " << OptInt << " == " << other.OptInt << std::endl
                    << "OptAddr -- " << OptAddr.ToString() << " == " 
                                     << other.OptAddr.ToString() << std::endl
                    << "OptEnable -- " << OptEnable << " == " << other.OptEnable << std::endl
                    << "OptSet -- " << OptSet << " == " << other.OptSet << std::endl
                    << "OptExec.ExecNoParam -- " << OptExec.ExecNoParam << " == " 
                                                 << other.OptExec.ExecNoParam << std::endl
                    << "OptExec.ExecStr -- " << OptExec.ExecStr << " == " 
                                             << other.OptExec.ExecStr << std::endl
                    << "OptHelp -- " << OptHelp << " == " << other.OptHelp << std::endl;
                
                for (size_t i = 0; i < other.OptExec.ExecList.size(); ++i)
                {
                    std::cout << "OptExec.ExecList(" << i << ") -- " 
                        << OptExec.ExecList.at(i) << " == " 
                        << other.OptExec.ExecList.at(i) << std::endl;
                }
            }

            return ((OptStr == other.OptStr)
                    && (OptQStr == other.OptQStr)
                    && (OptInt == other.OptInt)
                    && (OptInt == other.OptInt)
                    && (OptEnable == other.OptEnable)
                    && (OptSet == other.OptSet)
                    && (OptExec.IsSame(other.OptExec))
                    && (OptHelp == other.OptHelp));
        }
    };
    
    // ---------------------------------------------------

    const std::string CKey = "\\";
    
    const std::string CFlagNoParamSh = "np";
    const std::string CFlagNoParamL = "no_param";

    const std::string CFlagExecListSh = "lp";
    const std::string CFlagExecListL = "list_param";
    
    const std::string CFlagExecStringSh = "str";
    const std::string CFlagExecStringL = "str_param";

    const std::string CFlagStringSh = "s";
    const std::string CFlagStringL = "str2";

    const std::string CFlagQStringSh = "q";
    const std::string CFlagQStringL = "qstr";

    const std::string CFlagIntSh = "i";
    const std::string CFlagIntL = "int";

    const std::string CFlagHostInfSh = "a";
    const std::string CFlagHostInfL = "addr";

    const std::string CFlagBoolSh = "e";
    const std::string CFlagBoolL = "enable";

    const std::string CFlagBoolWithParamSh = "set";     // short and long name     
    const std::string CFlagBoolWithParamL = "set2";      // can't be the same

    const std::string CFlagTmpSh = "l";
    const std::string CFlagTmpL = "local";

    class TestConfig
    {
        typedef TestConfig T;
        
        ConfigData m_data;
        bool m_helpRequested;
        QString m_help;

        void ExecNoParam() 
        {
            m_data.OptExec.ExecNoParam = true; 
        }
        
        void ExecString(const std::string &s) 
        { 
            m_data.OptExec.ExecStr = s; 
        }
        
        void ExecList(const std::vector<std::string> &v) 
        { 
            m_data.OptExec.ExecList.clear();

            for (size_t i = 0; i < v.size(); ++i)
            {
                m_data.OptExec.ExecList.push_back(v.at(i));
            }
        }

        static std::vector<std::string> vec(const std::string &first, 
                                            const std::string &second = "",
                                            const std::string &third = "")
        {
            std::vector<std::string> res;
            res.push_back(first);
            if (!second.empty()) res.push_back(second);
            if (!third.empty()) res.push_back(third);
            return res;
        }
    
    public:
    
        TestConfig(const Utils::StringList &args)
        {
            Utils::ProgramOptions<TestConfig> opt(*this, CKey);

            opt.AddDefaultHelpOption();
            
            opt.Add(&T::ExecNoParam, vec(CFlagNoParamSh, CFlagNoParamL), "no param");
            opt.Add(&T::ExecList, vec(CFlagExecListSh, CFlagExecListL), "list param");

            // check that short name for one flag can be long name for other
            opt.Add(&T::ExecString, vec(CFlagExecStringSh, CFlagExecStringL), "str param");
            opt.Add(SetOptString(m_data.OptStr, vec(CFlagStringSh, CFlagStringL), "string"));

            opt.Add(SetOptQString(m_data.OptQStr, vec(CFlagQStringSh, CFlagQStringL), "qstring"));
            opt.Add(SetOptInt(m_data.OptInt, vec(CFlagIntSh, CFlagIntL), "int"));
            opt.Add(SetOptHostInf(m_data.OptAddr, vec(CFlagHostInfSh, CFlagHostInfL), "host interface"));
            opt.Add(SetOptBool(m_data.OptEnable, vec(CFlagBoolSh, CFlagBoolL), "bool"));
            opt.Add(SetOptBool(m_data.OptSet, vec(CFlagBoolWithParamSh, CFlagBoolWithParamL), "bool"));

            // test with local variable
            std::string tmp;
            opt.Add(SetOptString(tmp, vec(CFlagTmpSh, CFlagTmpL), "local string"));

            opt.Process(args);

            m_helpRequested = opt.HelpRequested();
            m_help = QString::fromStdString(opt.HelpInfo());
        }

        void TestDublicateNameErr()
        {
            // For Add with functions
            {
                bool wasException = false;
                Utils::ProgramOptions<TestConfig> opt(*this);
                opt.Add(&T::ExecNoParam, vec(CFlagNoParamSh, CFlagNoParamL), "no param");
                try 
                { 
                    opt.Add(&T::ExecNoParam, vec(CFlagNoParamSh, CFlagNoParamL), "no param"); 
                }
                catch (ProgramOptionsExceptions::DublicateName &e) { wasException = true; }
                TUT_ASSERT(wasException);
            }
            // For Add with setters
            {
                bool wasException = false;
                Utils::ProgramOptions<TestConfig> opt(*this);
                opt.Add(SetOptString(m_data.OptStr, vec(CFlagStringSh, CFlagStringL), "string"));
                try 
                { 
                    opt.Add(SetOptString(m_data.OptStr, vec(CFlagStringSh, CFlagStringL), "string"));
                }
                catch (ProgramOptionsExceptions::DublicateName &e) { wasException = true; }
                TUT_ASSERT(wasException);
            }
            // Both long names is empty --> no error
            {
                Utils::ProgramOptions<TestConfig> opt(*this);
                opt.Add(SetOptString(m_data.OptStr, vec(CFlagStringSh), "string"));
                opt.Add(SetOptString(m_data.OptStr, vec(CFlagStringL), "string"));
            }
        }

        const ConfigData& Data() const { return m_data; }

        bool HelpRequested() const { return m_helpRequested; }
        const QString& Help() const { return m_help; }
    };

    // ------------------------------------------------------

    QString Key(bool useShort, const std::string &shortKey, const std::string &longKey)
    {
        QString res;
        if (useShort)
        { 
            res.append(QString::fromStdString(CKey))
                .append(QString::fromStdString(shortKey));
        }
        else
        { 
            res.append(QString::fromStdString(CKey))
                .append(QString::fromStdString(longKey));
        }
        
        return res;
    }
    
    // ------------------------------------------------------

    // shortKey == false -- using longKey
    Utils::StringList MakeCmdLine(const ConfigData &src, bool silentMode, bool shortKey)
    {
        Utils::StringList res;
        res << "exeName" << Key(shortKey, CFlagTmpSh, CFlagTmpL) << "local";
        if (src.OptExec.ExecNoParam == true)
        {
            res << Key(shortKey, CFlagNoParamSh, CFlagNoParamL);
        }
        if (!src.OptExec.ExecStr.empty())
        {
            res << Key(shortKey, CFlagExecStringSh, CFlagExecStringL) 
                << QString::fromStdString(src.OptExec.ExecStr);
        }
        if (src.OptExec.ExecList.size() != 0)
        {
            res << Key(shortKey, CFlagExecListSh, CFlagExecListL);
            for (size_t i = 0; i < src.OptExec.ExecList.size(); ++i)
            {
                res << QString::fromStdString(src.OptExec.ExecList.at(i)); 
            }
        }
        if (!src.OptStr.empty())
        {
            res << Key(shortKey, CFlagStringSh, CFlagStringL) 
                << QString::fromStdString(src.OptStr);
        }
        if (!src.OptQStr.isEmpty())
        {
            res << Key(shortKey, CFlagQStringSh, CFlagQStringL) << src.OptQStr;
        }
        if (src.OptInt > 0)
        {
            res << Key(shortKey, CFlagIntSh, CFlagIntL) << QString::number(src.OptInt);
        }
        if (src.OptAddr.IsValid())
        {
            res << Key(shortKey, CFlagHostInfSh, CFlagHostInfL) 
                << QString::fromStdString(src.OptAddr.ToString());
        }
        if (src.OptEnable)
        {
            res << Key(shortKey, CFlagBoolSh, CFlagBoolL);
        }
        if (src.OptHelp)
        {
            res << Key(shortKey, "h", "help");
        }

        QString setVal = (src.OptSet)? "1" : "0";
        res << Key(shortKey, CFlagBoolWithParamSh, CFlagBoolWithParamL) << setVal;

        if (!silentMode)
        {
            std::cout << res.join(" ") << std::endl;
        }

        return res;
    }

    // ------------------------------------------------------

    void TestAllOk(bool silentMode, bool useShortKey)
    {
        ConfigData d;
        d.OptStr = "string";
        d.OptQStr = "qstring";
        d.OptInt = 5;
        d.OptAddr = Utils::HostInf("125.3.6.15", 1598);
        d.OptEnable = true;
        d.OptSet = useShortKey; // to check and true, and false
        d.OptExec.ExecStr = "\"some exec_string\"";
        d.OptExec.ExecNoParam = true;
        d.OptExec.ExecList = std::vector<std::string>(5, "asdf");
        
        TestConfig t(MakeCmdLine(d, silentMode, useShortKey));
        TUT_ASSERT(d.IsSame(t.Data(), silentMode));
    }

    // ------------------------------------------------------

    void TestHelp(bool silentMode, bool useShortKey)
    {
        ConfigData d;
        d.OptStr = "string";
        d.OptHelp = true;
        
        TestConfig t(MakeCmdLine(d, silentMode, useShortKey));
        TUT_ASSERT(t.HelpRequested());
        if (!silentMode) std::cout << "Help test:" << t.Help() << std::endl;
    }

    // ------------------------------------------------------

    bool WithProcessError(const Utils::StringList &commandLine, bool silentMode)
    {
        try
        {
            TestConfig t(commandLine);
        }
        catch (ProgramOptionsExceptions::ProcessError &e)
        {
            if (!silentMode) std::cout << "Test error: " << e.getTextMessage() << std::endl;
            return true;
        }

        return false;
    }
    
    // ------------------------------------------------------

    void TestErrors(bool silentMode, bool useShortKey)
    {
        // dublicate flags in command line -- NO exception
        {
            Utils::StringList cmdLine;
            cmdLine << "exeName" << Key(useShortKey, CFlagNoParamSh, CFlagNoParamL) 
                    << Key(useShortKey, CFlagNoParamSh, "no_param");

            TUT_ASSERT(!WithProcessError(cmdLine, silentMode));
        }
        // flag without param (but need one)
        {
            // call function
            {
                Utils::StringList cmdLine;
                cmdLine << "exeName" << Key(useShortKey, CFlagNoParamSh, CFlagNoParamL) 
                    << Key(useShortKey, CFlagExecStringSh, CFlagExecStringL);
    
                TUT_ASSERT(WithProcessError(cmdLine, silentMode));
            }
            // setter
            {
                Utils::StringList cmdLine;
                cmdLine << "exeName" << Key(useShortKey, CFlagStringSh, CFlagStringL);
                
                TUT_ASSERT(WithProcessError(cmdLine, silentMode));
            }
        }
        // flag with param (but don't need one)
        {
            Utils::StringList cmdLine;
            cmdLine << "exeName" << Key(useShortKey, CFlagNoParamSh, CFlagNoParamL) << "text";

            TUT_ASSERT(WithProcessError(cmdLine, silentMode));
        }
        // no flag
        {
            Utils::StringList cmdLine;
            cmdLine << "exeName" << "text";

            TUT_ASSERT(WithProcessError(cmdLine, silentMode));
        }
        // invalid int flag value
        {
            Utils::StringList cmdLine;
            cmdLine << "exeName" << Key(useShortKey, CFlagIntSh, CFlagIntL) << "text";

            TUT_ASSERT(WithProcessError(cmdLine, silentMode));
        }
        // invalid HostInf flag value
        {
            Utils::StringList cmdLine;
            cmdLine << "exeName"  << Key(useShortKey, CFlagHostInfSh, CFlagHostInfL) << "text";

            TUT_ASSERT(WithProcessError(cmdLine, silentMode));
        }
        // unknown flag
        {
            Utils::StringList cmdLine;
            cmdLine << "exeName"  << Key(useShortKey, "uf", "unknown_flag");
            TUT_ASSERT(WithProcessError(cmdLine, silentMode));
        }
        // string flag with 2 params
        {
            Utils::StringList cmdLine;
            cmdLine << "exeName"  << Key(useShortKey, CFlagExecStringSh, CFlagExecStringL) 
                        << "text1" << "text2";
            TUT_ASSERT(WithProcessError(cmdLine, silentMode));
        }
        // invalid bool option value (can be <none>, 1 or 0)
        {
            Utils::StringList cmdLine;
            cmdLine << "exeName"  << Key(useShortKey, CFlagBoolWithParamSh, CFlagBoolWithParamL) 
                        << "5";
            TUT_ASSERT(WithProcessError(cmdLine, silentMode));
        }
    }

    // ------------------------------------------------------

    void AllTestsDependOnKey(bool silentMode, bool useShortKey)
    {
        TestAllOk(silentMode, useShortKey);
        TestHelp(silentMode, useShortKey);
        TestErrors(silentMode, useShortKey);
    }

    // ------------------------------------------------------

    
    
} // namespace 

// ------------------------------------------------------

namespace UtilsTests
{
    void ProgramOptionsTest(bool silentMode)
    {
        Utils::StringList sl;
        TestConfig t(sl);
        t.TestDublicateNameErr();
        
        AllTestsDependOnKey(silentMode, true);
        AllTestsDependOnKey(silentMode, false);

        if (!silentMode) std::cout << "ProgramOptionsTest: OK" << std::endl;
    }
    
} // namespace UtilsTests




