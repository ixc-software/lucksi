
#include "stdafx.h"

#include "iNetTests.h"
#include "TestTcp.h"            
#include "TestUdp.h"

namespace 
{
    const int CTimeoutMs = 3000;

} // namespace 

// ---------------------------------

namespace
{
	using namespace iNetTests;
	using namespace TestFw;

    class iNetTestsGroup : public TestGroup<iNetTestsGroup>
    {
        iNetTestsGroup(ILauncher &launcher) : TestGroup<iNetTestsGroup>(launcher, "iNet")             
        {
			bool silentMode = true;
            
			Add("Tcp test",	&TcpTest, silentMode);
            Add("Udp test", &UdpTest, silentMode);
        }

    public:

        static void AddToLauncher(ILauncher &launcher)
        {
            new iNetTestsGroup(launcher);  // free at launcher
        }
    };

} // namespace

// ------------------------------------------------

namespace iNetTests
{
    void RegisterTestGroup(TestFw::ILauncher &launcher)
    {
        iNetTestsGroup::AddToLauncher(launcher);
    }

} // namespace iNetTests

