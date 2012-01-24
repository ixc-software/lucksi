#include "stdafx.h"
#include "LogTest.h"
#include "iLog/iLogTestGroupExt.h"

namespace
{
    using namespace TestFw; 
    using namespace iLog;

    class TestsGroup : public TestGroup<TestsGroup>
    {
        template<class TGroup>
        friend void RegisterExtendedTests(TGroup &g);

        TestsGroup(ILauncher &launcher) 
            : TestGroup<TestsGroup>(launcher, "iLog")
        {
            Add("Simple wrapper test", &iLogWrapperSimpleTest, false);
            Add("Wrapper benchmark", &iLogWrapperBenchmarkAsTest);

			// register platform specific tests
            RegisterExtendedTests(*this);
        }

    public:

        static void Register(ILauncher &launcher)
        {
            new TestsGroup(launcher);  // free at launcher
        }
    }; 

}

// --------------------------------------------

namespace iLog
{
	
    void RegisterTestGroup(TestFw::ILauncher &launcher)
    {
        TestsGroup::Register(launcher); 
    }
		
}  // namespace iLog

