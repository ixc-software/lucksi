#include "stdafx.h"

// #include "Utils/SafeRef.h"
// #include "Domain/DomainTestRunner.h"
// #include "Domain/DomainClass.h"
// #include "iCore/MsgObject.h"
// #include "iLog/LogManager.h"

#include "DRI/MetaInvoke.h"
#include "DRI/DRICommandStreamParser.h"
#include "DRI/ParsedDRICmd.h"
#include "DRI/DRIParserHelpers.h"
#include "DRI/ObjNamesValidator.h"
#include "DRI/TransactionDRI.h"
#include "DRI/ExecutiveDRI.h"
#include "DRI/SessionDRI.h"
// #include "DRI/FileDRI.h"
#include "DRI/NamedObjectFactory.h"
// #include "DRI/Diff.h"
// #include "DRI/drifileload.h"

#include "DriTests/DRIParserTests.h" 
#include "DomainTests/DomainNamedObjectTests.h"
#include "DriTests/DRITestHelpers.h"


using namespace Domain;
using namespace DRI;
using namespace DriTests;
using namespace DriTestHelpers;

using boost::scoped_ptr;

// ------------------------------------------------------

namespace
{

    void ExtTests()
    {
        // domain
        {
            //DomainTests::NamedObjectTreeTest();
            //DomainTests::DublicateChildTest();
        }

        // parser
        {
            bool silenceMode = true;
            DriParserTests(silenceMode); 
        }
    }

}  // namespace

// ------------------------------------------------------

namespace
{

    struct ScriptRunParams
    {
        QString ScpFile;
        QString ScpRefOutput;
        bool SilenceMode;

        // output
        bool OK;

        ScriptRunParams() : SilenceMode(true), OK(true) {}
    };
    
}  // namespace

// ------------------------------------------------------

namespace
{
    struct TelnetParams
    {
        // ...
    };

    class TelnetTest
    {
    public:

        TelnetTest(DomainClass &domain, TelnetParams &params) 
        {
            SetupLog(domain);
        }

    };

    void RunTelnet()
    {
        Domain::DomainTestRunner runner;

        TelnetParams params;

        bool res = runner.Run<TelnetTest>(params);
        TUT_ASSERT(res);
    }


}  // namespace

// ------------------------------------------------------

namespace
{
    class FileLinesTestClass
    {
    public:
        FileLinesTestClass(Domain::DomainClass &domain, QString fileName)
        {
            Utils::StringList sl;
            sl.LoadFromFile(fileName);
    
            for(int i = 0; i < sl.size(); ++i)
            {
                std::cout << (i + 1) << ": " << sl.at(i).toStdString() << std::endl;
            }
    
            // as script
            DRI::DriIncludeDirList pathes;
            DRI::DRIFileLoad driFile(fileName, pathes, domain.Params().SpecialParams());
    
            while(true)
            {
                shared_ptr<ParsedDRICmd> cmd = driFile.Peek();
                if (cmd == 0) break;
    
                std::cout << cmd->OriginalCmd().toStdString() 
                    << " " << cmd->LocationInfo().toStdString() << std::endl;
            }

            domain.Stop(Domain::DomainExitOk);
        }
    };

    void FileLinesTest(QString fileName)
    {
        const Platform::dword CTestTimeoutMS = 10 * 1000;
        Domain::DomainTestRunner runner(CTestTimeoutMS);
        
        bool res = runner.Run<FileLinesTestClass>(fileName);
        TUT_ASSERT(res);
    }

    template<class T>
    void ArgInfo(const T &arg) 
    {
        std::cout << typeid(T).name() << " " << sizeof(arg) << std::endl;
    }

    void ArrayTest()
    {
        char buff[10];
        char buff2[16];

        ArgInfo(buff);
        ArgInfo(buff2);
    }
}

// ------------------------------------------------------

namespace DriTests
{
    
    void DriTest()
    {
        using namespace std;

        // FileLinesTest("D:/proj/Ipsius/ProjDomain/isc/TestCS_Commands.isc");

        // ArrayTest();

        // ExtTests();

        // Telnet
        // RunTelnet();

        // file 
        {

            DriTestsParams params;
            params.SilentMode = false;
            RunScript(params, "test1.isc", "test1_output.txt");  // 

            // RunScript(false, "exit_test.isc", "");
            // RunScript(false, "props.isc", "");

            // RunScript(false, "TestMain.isc", "TestMain_output.txt");
        }

        // done
        cout << "OK" << endl;

        char c;
        // cin >> c;
    }
        
}  // namespace DriTests

