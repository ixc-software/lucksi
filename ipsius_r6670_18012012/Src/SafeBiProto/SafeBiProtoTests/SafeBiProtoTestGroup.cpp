
#include "stdafx.h"
#include "SafeBiProtoTestGroup.h"

#include "SafeBiProto/SafeBiProtoTests/SafeBiProtoTests.h"

#include "SafeBiProto/SafeBiProtoTests/iNetTcpTransportsCreator.h"
#include "SafeBiProto/SafeBiProtoTests/TestTcpTransportCreator.h"
#include "SafeBiProto/SafeBiProtoTests/TestSpbTcpServer.h"

namespace 
{
    using namespace SBPTests;
    
	void TestGroupWithTcp(ITestTransportsCreator &creator, bool silentMode)
	{
		// with msg
		SafeBiProtoTestSimple(creator, true, silentMode);
		SafeBiProtoTest(creator, true, silentMode);

		// without
		SafeBiProtoTestSimple(creator, true, silentMode);
		SafeBiProtoTest(creator, true, silentMode);
	}

	// --------------------------------------------------------------

    void TestGroupWithiNetTcp(bool silentMode)
    {
        iNetTcpTransportsCreator creator;
		TestGroupWithTcp(creator, silentMode);
    }

    // --------------------------------------------------------------

    void TestGroupWithTestTcp(bool silentMode)
    {
        TestTcpTransportsCreator creator;
		TestGroupWithTcp(creator, silentMode);
    }


	// --------------------------------------------------------------
    
	using namespace Utils;
    using namespace UtilsTests;
	using namespace TestFw;

    class SafeBiProtoTestGroup : public TestGroup<SafeBiProtoTestGroup>
    {
        SafeBiProtoTestGroup(ILauncher &launcher) 
		: TestGroup<SafeBiProtoTestGroup>(launcher, "SafeBiProto")             
        {
            bool silentMode = true;
           
            Add("SafeBiProto utils tests",                  &SbpUtilsTests,        silentMode);
            Add("SafeBiProto parsing tests",                &SbpRecvParsingTest,   silentMode);
            Add("SafeBiProto tests with test Tcp sockets",  &TestGroupWithTestTcp, silentMode);
            Add("SafeBiProto tests with iNet::Tcp sockets", &TestGroupWithiNetTcp, silentMode);
            Add("SafeBiProto exceptions test",              &SbpExceptionsTest,     silentMode);
      }

    public:
        static void AddToLauncher(ILauncher &launcher)
        {
            new SafeBiProtoTestGroup(launcher);  // free at launcher
        }
    };

} // namespace 

// ------------------------------------------------

namespace SBPTests
{
    void RegisterTestGroup(TestFw::ILauncher &launcher)
    {
        SafeBiProtoTestGroup::AddToLauncher(launcher);
    }

} // namespace UtilsTests

