
#include "stdafx.h"
#include "Utils/ExeName.h"
#include "Utils/DelayInit.h"
#include "Utils/ArgStringList.h"
#include "FinalChecks.h"
#include "TestFw/TestFw.h"
#include "TestFw/TestFwOutput.h"
#include "TestFw/TestVerification.h"

// tests
#include "TestFw/TestFwGroup.h"
#include "iCoreTests/iCoreGroupBasic.h"
#include "iNetTests/iNetTests.h"
#include "TelnetTests/TelnetTestGroup.h"
#include "UtilsTests/ErrorsSubsystemTests.h"
#include "UtilsTests/UtilsTestGroup.h"
#include "UtilsTests/ErrorGenGroup.h"
#include "PlatformTests/PlatformTestsGroup.h"
#include "iLog/LogTest.h"
#include "SafeBiProto/SafeBiProtoTests/SafeBiProtoTestGroup.h"
#include "iDSP/EchoTestGroup.h"
#include "SndMix/MixerTest.h"
#include "DriTests/DriTestsGroup.h"

#include "iDSP/RegSpanTestsGroup.h"




class AppExceptionHook : public ESS::ExceptionHook
{ 
	int m_counter;

public:

	void Hook(const ESS::BaseException *pE)  // override
	{
        std::string msg = pE->what();

		if (dynamic_cast<const ESS::Assertion*>(pE)) 
		{ 
			m_counter++;
		}
		if (dynamic_cast<const ESS::TutException*>(pE))
		{
			m_counter++;
		}
	}
};

namespace 
{
	int CSingleTestTimeout = 480000;

	// -------------------------------------------------------
	
	void RegisterAllTests(TestFw::TestLauncher &launcher)
	{      
        TestFw::RegisterTestGroup(launcher);
        // TestFw::AdvancedTests::RegisterTestGroup(launcher);
        
        UtilsTests::ESSTests::RegisterTestGroup(launcher);
        UtilsTests::ErrorGen::RegisterTestGroup(launcher);
        PlatformTests::RegisterTestGroup(launcher);
        UtilsTests::RegisterTestGroup(launcher);
        iCoreTests::RegisterTestGroup(launcher);
        iLog::RegisterTestGroup(launcher);
        iNetTests::RegisterTestGroup(launcher);
        TelnetTests::RegisterTestGroup(launcher);
        SBPTests::RegisterTestGroup(launcher);
        iDSP::RegisterTestGroup(launcher);
        DriTests::RegisterTestGroup(launcher);      
        iDSP::RegSpanTestsGroup(launcher);     
        SndMix::RegisterTestGroup(launcher);
	}

} // namespace 

// -------------------------------------------------------

int main(int argc, char* argv[])
{
    // create thread to run "thread manager" once
    //iCore::MsgThread m(QThread::LowestPriority, true);

	ESS_ASSERT(argc > 0);
    Utils::ExeName::Init(argv[0]);

    Utils::DelayInitHost::Inst().DoInit();
	QCoreApplication a(argc, argv);  
	AppExceptionHook hook;

	std::cout << "IPSIUS Project tests:\n";

    Utils::ArgStringList strList(argc, argv);
    TestFw::StdOutput output;
	TestFw::TestVerification testVer(CSingleTestTimeout);
    TestFw::TestLauncher launcher(output);
    launcher.AddOutputter(testVer);

	RegisterAllTests(launcher);

	launcher.Run(strList);	
    
    // Do final checks
    FinalChecks();  

    return 0;
} 


