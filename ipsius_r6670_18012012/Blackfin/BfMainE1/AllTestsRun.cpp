#include "stdafx.h" 
#include "AllTestsRun.h"

#include "TestFw/TestFw.h"
#include "TestFw/TestFwGroup.h"
#include "TestFw/TestFwOutput.h"

#include "PlatformTests/PlatformTestsGroup.h"

#include "Utils/ErrorsSubsystem.h"
#include "Utils/ErrorsSubsystemTests.h"
#include "Utils/VirtualInvokeTest.h"
#include "Utils/BidirBuffer.h"
#include "Utils/MemoryPool.h"
#include "Utils/TimerBase.h"
#include "Utils/TimerTicks.h"
#include "Utils/DateTimeCapture.h"
#include "Utils/DeleteIfPointer.h"
#include "Utils/ArgStringList.h "

// #include "PlatformTests/PlatformThreadTest.h"
#include "PlatformTests/PlatformTests.h"

#include "iCore/MsgThread.h"
#include "iCoreTests/iCoreGroupBasic.h"
#include "iCoreTests/iCoreSimpleMsgTest.h"
#include "iCoreTests/iCoreTimerTest.h"
#include "iCoreTests/iCoreMsgBenchmarks.h"

#include "boost/detail/BoostTestGroup.h"
#include "iLog/LogTest.h"

#include "BfDev/SysProperties.h"
#include "BfDev/TicksVisualTest.h"
#include "BfDev/AssertTest.h"

#include "Bftdm/tdmtest.h" 

#include "../BfPort/SpecializationTest.h"
#include "../BfPort/RefTest.h"

#include "SafeBiProto/SafeBiProtoTests/SafeBiProtoTests.h"
#include "SafeBiProto/SafeBiProtoTests/TestTcpTransportCreator.h"

#include "UtilsTests/MemoryPoolTests.h"
#include "UtilsTests/BinaryReaderWriterTests.h"
#include "UtilsTests/MemReaderWriterStreamTests.h"



// ----------------------------------------------------

namespace
{
	void BoostTests()
	{
		boost::detail::shared_ptr_test(true);
    	boost::detail::scoped_ptr_test(true);		
	}
	
	void ExtTests()
	{
		UtilsTests::MemBlockPoolTest();
    	UtilsTests::MemBlockPoolTestSimulation();

	    Utils::DeletePointerTest();
	    UtilsTests::VirtualInvokeTest();
	
		BoostTests();   	
		
	    PlatformTests::WaitConditionTest();
	    PlatformTests::MutexTest();        
	    PlatformTests::ThreadTest();	
	}

	void MsgThreadInstanceTest()
	{
			iCore::MsgThread thread;
			thread.Run();
			Platform::ThreadSleep(10);
			thread.Break();	
	}

	void iCoreTestsRun()
	{
		// Debug,     I   cache    -- ~15K	
		// Debug,     I+D cache    -- ~50K
		// Speed opt, I+D cache    -- ~150K
		std::string res = iCoreTests::iCoreBenchmarkSingle(250);	
		std::cout << res << std::endl;
		
		iCoreTests::iCoreTimerTest();
		
		iCoreTests::iCoreSimpleMsgTest();
		
		MsgThreadInstanceTest();	
	}
	
	void TestFwRun()
	{
	    Utils::ArgStringList sl;
	    TestFw::StdOutput output;
	    TestFw::TestLauncher launcher(output);
	
	    {   
	        TestFw::RegisterTestGroup(launcher);
	        UtilsTests::ESSTests::RegisterTestGroup(launcher);
	        // iLog::RegisterTestGroup(launcher);   // out of heap (?)
	        boost::detail::RegisterTestGroup(launcher);         
	        PlatformTests::RegisterTestGroup(launcher);     	
	        iCoreTests::RegisterTestGroup(launcher);        
	    	 	                                
	        // ...
	    }
	
	    launcher.Run(sl);		
	}

	BOOST_STATIC_ASSERT(sizeof(int) == sizeof(long));
	BOOST_STATIC_ASSERT(sizeof(long long) == 8);
		
}  // namespace

// ----------------------------------------------------

void ExecuteAllTests()
{	
	ExtTests();	
	iCoreTestsRun();	
	TestFwRun();
	
	std::cout << "Done!" << std::endl;
	
}

// ----------------------------------------------------

namespace
{
	using namespace SBPTests;
	
	void TestWithTransportsGroup(ITestTransportsCreator &creator, bool silentMode)
    {
        SafeBiProtoTestSimple(creator, silentMode);
        SafeBiProtoTest(creator, silentMode);
    }
    
    void RunSbpTest()
    {
		bool silentMode = false;
	
	    UtilsTests::MemReaderWriterStreamTests();
	    UtilsTests::BinaryReaderWriterTests(silentMode);

	    SbpUtilsTests(silentMode);
	    SbpRecvParsingTest(silentMode);    		
    
	    TestTcpTransportsCreator testTcpCreator;
	    TestWithTransportsGroup(testTcpCreator, silentMode);    	
    }

}  // namespace

void ExecuteSafeBiProtoTest()
{
	RunSbpTest();
	
	std::cout << "SBP Done!" << std::endl;	
}

// ----------------------------------------------------

void ExecuteAssertTest()
{
	BfDev::RunAssertTest();	
}

// ----------------------------------------------------

/*

    void TdmFramer::TdmTest()
    {
    	
		int CTestRunMs = 10;	
		std::cout << "Start TDM Test (" << CTestRunMs << "sec )"<< std::endl;	
		CTestRunMs *= 1000;	

		BfTdm::TdmTestResult res;

		BfTdm::TdmTest test((Platform::byte *)0x20300000UL, CTestRunMs, 8, res);

		while(true)
		{
			iVDK::Sleep(1000);
			if (test.Polling()) break;
		}

		std::cout << res.AsString << std::endl;
		ESS_ASSERT(res.IsPassed);    	    	
    }


*/


void ExecuteTdmTest()
{			
	int CTestRunMs = 10;	
	std::cout << "Start TDM Test (" << CTestRunMs << "sec )"<< std::endl;	
	CTestRunMs *= 1000;	

	BfTdm::TdmTestResult res;
	
	BfTdm::TdmTest test(Ds2155::CBaseAddress, CTestRunMs, 8, res);
	
	Utils::TimerTicks t;
	t.Set(CTestRunMs * 2);
	
	while(true)
	{
		iVDK::Sleep(1000);
		if (test.Polling()) break;
		
		if (t.TimeOut()) ESS_HALT("Timeout");
	}
	
	std::cout << res.AsString << std::endl;
	ESS_ASSERT(res.IsPassed);
}


