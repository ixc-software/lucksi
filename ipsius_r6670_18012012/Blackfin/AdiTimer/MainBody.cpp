#include "stdafx.h" 
#include "Utils/DelayInit.h"
#include "AdiTimerTestRun.h" 

void MainBody()
{
	AdiTest::RunThreadTest();
}

int main(void) 
{		
	iVDK::VdkInitDone();
	Utils::DelayInitHost::Inst().DoInit();
	BfDev::SysProperties::InitSysFreq133000kHz();
	
	AdiTest::Run();
    
    VDK::Initialize();    
    VDK::Run();     
}

