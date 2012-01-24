#include "stdafx.h"

#include "PlatformTestsGroup.h"
#include "PlatformTests.h"
#include "ThreadWaitUSTest.h"
#include "ThreadCounterTest.h"
#include "FormatTypeidNameTest.h"

// ------------------------------------------

namespace 
{
	using namespace TestFw;
    using namespace PlatformTests;
    
    class PlatformTestsGroup : public TestGroup<PlatformTestsGroup>
    {

        PlatformTestsGroup(ILauncher &launcher) 
		    : TestGroup<PlatformTestsGroup>(launcher, "Platform")             
        {
            bool printInfo = false;

            Add("Us-sleep test in Win32",	&ThreadWaitUSTest);
            Add("Min thread's sleep test", 	&ThreadMinimalSleepTest);
			Add("Thread's counter test",	&GetCurrentProcessThreadCountTest, printInfo);
            Add("TypeIdName's parser test", &FormatTypeidNameTest);                       
            Add("Thread test",              &ThreadTest);
            Add("Mutex test",               &MutexTest);
            Add("WaitCondition test",       &WaitConditionTest);
       }

    public:

        static void AddToLauncher(ILauncher &launcher)
        {
            new PlatformTestsGroup(launcher);  // free at launcher
        }
    };

} // namespace 

// ------------------------------------------------

namespace PlatformTests
{
    void RegisterTestGroup(TestFw::ILauncher &launcher)
    {
        PlatformTestsGroup::AddToLauncher(launcher);
    }

} // namespace PlatformTests

