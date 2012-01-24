#include "stdafx.h"

#include "BfDev/SysProperties.h"
#include "iDSP/EchoTestBf.h"
#include "iDSP/EchoTest.h"
#include "Utils/RawCRC32.h"
#include "iVDK/VdkCheckInit.h"

#include "EchoApp/LowBench.h"

#include "FlashTestLaunch.h"

// -----------------------------------------------------------------------------

void RunEchoTest()
{
    iDSP::RunEchoTestBf();
    return;
    
    Utils::TestRawCrc32();
   
    {
        iDSP::EchoTest::GeneratorTest();
        iDSP::EchoTest::RunReferenceTest();

        iDSP::EchoTest::Run(128, 32, 25);
        iDSP::EchoTest::Run(128, 64, 25);
        iDSP::EchoTest::Run( 64, 64, 25);
        iDSP::EchoTest::Run( 32, 64, 25);
        
        std::cout << "Done!" << std::endl;
    }	
}

// -----------------------------------------------------------------------------


void MainBody()
{
    iVDK::VdkInitDone();

    BfDev::SysProperties::InitSysFreq133000kHz(); //	InitFromProjectOptions();
    
	// freq info
	{
		int freq = BfDev::SysProperties::Instance().getFrequencyCpu();
		std::cout << "Freq: " << (freq / (1000 * 1000)) << " MHz" << std::endl;		
	}
    
    // FlashTestLaunch();
    
    // RunEchoTest();
    
    // EchoApp::RunLowBenchmark();
    EchoApp::RunEchoCodingBench();

    std::cout << "Done!" << std::endl;
                
    int dummy = 0;
    
    while(true);
}
