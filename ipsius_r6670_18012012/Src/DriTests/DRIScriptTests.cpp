
#include "stdafx.h"

#include "DRITestHelpers.h"
#include "DRI/DriDefineAndMacroList.h"
#include "Utils/IStoragable.h"
#include "Utils/StringList.h"
#include "Utils/AtomicTypes.h"


namespace
{
    const Platform::dword CTestTimeoutMS = 10 * 1000;
    
} // namespace

// -----------------------------------------------------------------

namespace
{
    using namespace DRI;
    using namespace Utils;
    using namespace DriTests;
    
    // -----------------------------------------------------------------
    // Test DRI file preprocessor exceptions
    class TestExceptions
    {
        Domain::IDomain &m_domain;
        DriTestsParams m_params;        
        
    public:
        TestExceptions(Domain::IDomain &domain, 
                       const DriTestsParams &params, 
                       const QString &testHeader) : 
        m_domain(domain), m_params(params)
        {
            if (!m_params.SilentMode) std::cout << std::endl << testHeader << std::endl << std::endl; 
        }
        
        void Run(const QStringList &block, 
                 const QString &expectedErr, 
                 const QString &macroReplaceList = "")
        {
            // run script
            try
            {
                DriIncludeDirList pathes(m_params.ScriptDirs);
                TUT_ASSERT(pathes.Verify());

                QStringList specParams;
                DRIFileLoad loader(block, pathes, 
                                   specParams,
                                   macroReplaceList);
            }
            catch(ESS::BaseException &e)
            {
                if (!m_params.SilentMode) std::cout << e.getTextMessage() << std::endl;

                std::string err = Platform::FormatTypeidName(typeid(e).name());
                TUT_ASSERT(err == expectedErr.toStdString());
                return;
            }            
            
            TUT_ASSERT(0 && "No exception!");
        }

        void Run(const QString &line, 
                 const QString &expectedErr, 
                 const QString &macroReplaceList = "")
        {
            QStringList block(line);
            Run(block, expectedErr, macroReplaceList);
        }
    };

    // -----------------------------------------------------------------

    // #include exceptions

    void TestIncludeExceptions(Domain::IDomain &domain,
                               const DriTestsParams &params)
    {
        const QString CInvalidPathErr = "DRI::DriIncludeDirList::InvalidPath";
        const QString CIncludeErr = "DRI::DRIFileLoad::DriIncludeError";
        
        TestExceptions t(domain, params, "Stage: #include test (with exceptions) ---------- ");
        

        // error: No prameter
        t.Run("#include",                                   CIncludeErr);

        // error: Recursive include
        t.Run("#include \"TestIncludeRecursive.isc\"",      CIncludeErr);

        // error: Path not found
        t.Run("#include <\"TestIncludeRecursive.isc\">",    CInvalidPathErr);
        t.Run("#include \"<TestIncludeRecursive.isc>\"",    CInvalidPathErr);
        t.Run("#include <TestIncludeRecursive.isc",         CInvalidPathErr);
        t.Run("#include \"TestIncludeRecursive.isc",        CInvalidPathErr);
        t.Run("#include \"_unknown_file_\"",                CInvalidPathErr);
        t.Run("#include \"#define\"",                       CInvalidPathErr);
        t.Run("#include \"file.isc\" flags",                CInvalidPathErr);
        {
            QStringList sl;
            sl.append("#define test_file ../isc/TestIncludeRecursive.isc");
            sl.append("#include %test_file%");  // macro-replace won't work here
            t.Run(sl, CInvalidPathErr);
        }

        if (!params.SilentMode) std::cout << "TestIncludeExceptions: OK" << std::endl;

    }

    // -----------------------------------------------------------------

    // #define / #undef exceptions
    void TestDefineExceptions(Domain::IDomain &domain, 
                              const DriTestsParams &params)
    {
        const QString CDefineErr = "DRI::DriDefineAndMacroList::DriDefineError";
        const QString CMacroErr = "DRI::DriDefineAndMacroList::DriMacroError";

        
        TestExceptions t(domain, params, "Stage: #define/#undef test (with exceptions) ---- ");


        // error: no name
        t.Run("#undef",                             CDefineErr);

        // error: invalid name
        t.Run("#undef _unknown_define_",            CDefineErr);
        t.Run("#define #define smth",               CDefineErr);
        t.Run("#define #include smth",              CDefineErr);
        t.Run("#define 2name smth",                 CDefineErr);
        t.Run("#define \"long comment\" \"// long coomment\"",
              CDefineErr);
        {
            QStringList sl;
            sl.append("#define smth smth2");		                // OK
            sl.append("#undef %smth%");						        // error
            t.Run(sl,                               CDefineErr);
        }

        // error: No params
        t.Run("#define",                            CDefineErr);

        // error: Unknown defined name
        t.Run("CS_Set %unknown_define%",            CDefineErr);
        t.Run("CS_Set \"%unknown_define%\"",        CDefineErr);
        // Using defines in external file after they undefined.
        t.Run("#include \"TestDefineExtern.isc\"",  CDefineErr);
        {
            QStringList sl;
            sl.append("#define comment \"//comment\"");		        // OK
            sl.append("%comment%%_unknown_define_%");		        // error
            t.Run(sl,                               CDefineErr);
        }

        // error: Value contains '#'
        t.Run("#define _include_file_ #include file.txt",
              CDefineErr);
        t.Run("#define _alt_define_ #define",       CDefineErr);

        // error: Dublicate name
        {
            QStringList sl;
            sl.append("#define comment smth");                      // OK
            sl.append("#define comment smth");					    // error
            t.Run(sl,                               CDefineErr);
        }

        // error: Trying undefine macross
        t.Run("#undef macro1", CMacroErr, "macro1 = text");

        if (!params.SilentMode) std::cout << "TestDefineExceptions: OK" << std::endl;
    }
    
} // namespace

// -----------------------------------------------------------------

namespace 
{
    class TestExceptionsParams
    {
        DriTestsParams m_params;

        Utils::AtomicBool m_testErr;
        // Utils::AtomicBool m_noScriptFilesErr;


    public:
        TestExceptionsParams(const DriTestsParams &params) :
            m_params(params), m_testErr(false)/*, m_noScriptFilesErr(false)*/
        {
        }

        const DriTestsParams& Params() const { return m_params; }
        const Utils::AtomicBool& TestErr() const { return m_testErr; }
        // const Utils::AtomicBool& NoScriptFilesErr() const { return m_noScriptFilesErr; }

        void TestFailed() { m_testErr.Set(true); }
        // void NoScriptFiles() { m_noScriptFilesErr.Set(true); }
    };

    // -----------------------------------------------------------------

    class TestExceptionsRunner
    {
    public:
        TestExceptionsRunner(Domain::DomainClass &domain, TestExceptionsParams &params)
        {
            try
            {
                TestDefineExceptions(domain, params.Params());
                TestIncludeExceptions(domain, params.Params());                
            }
            /*catch(DriIncludeDirList::InvalidPath &e)
            {
                // std::cout << "dri error: " << e.getTextMessage() << std::endl;
                params.NoScriptFiles();
            }*/
            catch (ESS::TutException &e)
            {
                params.TestFailed();
            }

            domain.Stop(Domain::DomainExitOk);
        }
    };
    
} // namespace 

// -----------------------------------------------------------------

void fn(int i)
{
}

namespace DriTests
{
    void TestIncludeAndDefineExceptions(DriTestsParams params)
    {
        Domain::DomainTestRunner runner(CTestTimeoutMS);
        TestExceptionsParams p(params);

        bool res = runner.Run<TestExceptionsRunner>(p);
        TUT_ASSERT(res);

        // if (p.NoScriptFilesErr()) TUT_ASSERT(0 && "Script files wasn't found");
        if (p.TestErr()) TUT_ASSERT(0 && "Test failed");
    }

    // -----------------------------------------------------------------

    void TestCmdLineSpecialParams(DriTestsParams params)
    {
        /* Don't change params and params order */
        QStringList specParams;
        specParams << "true" << "spObj11" << "spObj11.spObj111" 
                << "1024" << "0xAF" << "AF" << "127.10.20.85:56001";
        
        DriTestHelpers::RunScript(params, "TestSpecParams.isc", 
                                  "TestSpecParams_output.txt", specParams);
        
        if (!params.SilentMode) std::cout << "TestCmdLineSpecialParams: OK" << std::endl;
    }

    // -----------------------------------------------------------------

    void TestCS_Commands(DriTestsParams params)
    {
        DriTestHelpers::RunScript(params, "TestCS_Commands.isc", 
                                  "TestCS_Commands_output.txt");
    }
    
    // -----------------------------------------------------------------

    void TestObjCommands(DriTestsParams params)
    {
        DriTestHelpers::RunScript(params, "TestObjCommands.isc", 
                                  "TestObjCommands_output.txt");
    }
    
    // -----------------------------------------------------------------

    void TestMetaCommands(DriTestsParams params)
    {
        DriTestHelpers::RunScript(params, "TestMetaCommands.isc", 
                                  "TestMetaCommands_output.txt");
    }
    
    // -----------------------------------------------------------------

    void TestIncludeAndDefine(DriTestsParams params)
    {
        DriTestHelpers::RunScript(params, "TestIncludeAndDefine.isc", 
                                  "TestIncludeAndDefine_output.txt");
    }
    
    // -----------------------------------------------------------------

    void TestTest1(DriTestsParams params)
    {
        DriTestHelpers::RunScript(params, "test1.isc", "test1_output.txt");
    }

    // -----------------------------------------------------------------

    void TestSec(DriTestsParams params)
    {
        DriTestHelpers::RunScript(params, "DriSecTest.isc", "DriSecTest_output.txt");
    }

    void TestSecProfile(DriTestsParams params)
    {
        DriTestHelpers::RunScript(params, "DriSecTestProfile.isc", "DriSecTestProfile_output.txt");
    }

    // -----------------------------------------------------------------
    
    void DriScriptTests(const DriTestsParams &params)
    {
        TestIncludeAndDefineExceptions(params);
        TestCmdLineSpecialParams(params);
        
        TestCS_Commands(params);
        TestObjCommands(params);
        TestMetaCommands(params);
        TestIncludeAndDefine(params);

        // call after all other scripts, 
        // because it doesn't delete all created inside objects at the end
        TestTest1(params);
    }
    
} // namespace DriTests
