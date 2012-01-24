
#include "stdafx.h"

#include "BuildInfo.h"   // add auto update!

#include "Utils/ErrorsSubsystem.h"
#include "Utils/DelayInit.h" 

#include "BfDev/SysProperties.h"

#include "E1App/AppPlatformHook.h"
#include "EchoApp/EchoAppMain.h"
#include "EchoApp/SfxProto.h"

#include "EchoChip.h"

//#include "src/iDSP/TestDtmf.h"
//#include "src/iDSP/DtmfBenchmark.h"


// ------------------------------------------------------------

namespace
{

	void RunEchoApp()
	{
		EchoApp::EchoAppProfile profile;
		profile.BuildInfo = AutoBuildInfo::FullInfo();
		
		EchoApp::RunEchoApp(profile);
	}
	
}; // namespace

// ------------------------------------------------------------

void MainBody()
{
    // DevIpTdm::BfLed::SetColor(DevIpTdm::GREEN); 
        
    iVDK::VdkInitDone();
    BfDev::SysProperties::InitSysFreq133000kHz();
        
    Utils::DelayInitHost::Inst().DoInit();
    // DevIpTdm::BfLed::Set(1, true);                 
    
    // UART + no reboot
    E1App::AppPlatformHookSetup(true, true, false, EchoApp::CSfxProtoUartSpeed); 
    
    //iDSP::TestDtmf(false);
    //ESS_HALT("Completed!"); 
    
    //iDSP::DtmfBenchmark();
    //ESS_HALT("Completed!");
                
    // run
    {
	    //BFEchoChip::EchoChip::Run(); // old Echo
	    RunEchoApp();    	
    }
        
    // stop
    {
        std::cout << "Stop!" << std::endl;
        while(true) iVDK::Sleep(50);            
    }
    
    ESS_HALT("Completed!"); 
}

