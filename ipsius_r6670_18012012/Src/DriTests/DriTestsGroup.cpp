
#include "stdafx.h"

#include "DriTestsGroup.h"
#include "DRIScriptTests.h"
#include "DriIncludeDirListTest.h"
#include "DRIParserTests.h"

namespace
{
    using namespace TestFw;
	using namespace DriTests;

    class DriTestsGroup : public TestGroup<DriTestsGroup>
    {
        DriTestsGroup(ILauncher &launcher) : 
            TestGroup<DriTestsGroup>(launcher, "DRI")             
        {
			DriTests::DriTestsParams params;
            params.SilentMode = true;
            params.ScriptDirs = "../isc/;";

            Add("Test command parser",       &DriParserTests,                 params.SilentMode);
            Add("Test DriIncludeDirList",    &DriIncludeDirListTest,          params.SilentMode);
            Add("Test CS_-commands",         &TestCS_Commands,                params);
            Add("Test Obj-commands",         &TestObjCommands,                params);
            Add("Test Meta-commands",        &TestMetaCommands,               params);
            Add("Test #include and #define", &TestIncludeAndDefine,           params);
            Add("Test #include and #define exceptions",
                                             &TestIncludeAndDefineExceptions, params);
            Add("Test command line special params",
                                             &TestCmdLineSpecialParams,       params);
            Add("Test Test1",                &TestTest1,                      params);
            Add("Test security",             &TestSec,                        params);
            Add("Test security profile",     &TestSecProfile,                 params);
        }

    public:
        static void AddToLauncher(ILauncher &launcher)
        {
            new DriTestsGroup(launcher);  // free at launcher
        }
    };

} // namespace

// ------------------------------------------------

namespace DriTests
{
    void RegisterTestGroup(TestFw::ILauncher &launcher)
    {
        DriTestsGroup::AddToLauncher(launcher);
    }


} // namespace DriTests



