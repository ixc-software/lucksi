
#include "stdafx.h"

#include "iDSP/g711tests.h"
#include "iDSP/EchoTest.h"

#include "iDSP/EchoTestGroup.h"

// ------------------------------------------

namespace 
{
	using namespace TestFw;
    using namespace iDSP;
    
    class EchoTestsGroup : public TestGroup<EchoTestsGroup>
    {

        EchoTestsGroup(ILauncher &launcher) 
		    : TestGroup<EchoTestsGroup>(launcher, "Echo")             
        {
            Add("G711 test",    &G711RunTests);
            Add("Echo test",    &EchoTest::RunReferenceTest);
        }

    public:

        static void AddToLauncher(ILauncher &launcher)
        {
            new EchoTestsGroup(launcher);  // free at launcher
        }
    };

} // namespace 

// ------------------------------------------------

namespace iDSP
{
    void RegisterTestGroup(TestFw::ILauncher &launcher)
    {
        EchoTestsGroup::AddToLauncher(launcher);
    }

} // namespace Echo
