
#include "stdafx.h"

#include "Utils/ErrorsSubsystem.h"
#include "Utils/DelayInit.h"
#include "BfDev/SysProperties.h"
#include "MngLwip.h"

#include "AppConfig.h"
#include "AppCpuUsage.h"
#include "BfDev/BfUartTest.h"

// ------------------------------------------------------------

namespace
{
	
	class AppExceptionHook : public ESS::ExceptionHook
	{
	    int m_counter;

	public:

	    void Hook(const ESS::BaseException *pE)  // override
	    {
	        if (dynamic_cast<const ESS::Assertion*>(pE))
	        {
	            std::string info = pE->what();
	            m_counter++;
	            m_counter++;	            
	        }
	    }
	};

    // --------------------------------------------------------

	extern "C"
	{
		int lwip_GetBuildVersion();
	}
    
    void InitNetwork(bool logOn)
    {
        using LwIP::Stack;
        using namespace std;

        int heap = heap_space_unused(0);

        LwIP::NetworkSettings settings;
        AppConfig::GetNetworkSettings(settings);
        Stack::Init(settings);

        Stack::Instance().Establish();

        if (logOn)
        {
            cout << "IP: " << Stack::Instance().GetIP() << endl;       

            if (Stack::HwEmulationMode()) cout << "Network emulation!" << endl;

            heap = heap - heap_space_unused(0);
            cout << "Lw heap alloc: " <<  heap << endl; 
        }
        
        ESS_ASSERT(lwip_GetBuildVersion() >= 2);
        // cout << "Max socks " << lwip_GetMaxSocks() << endl;        	 
    }
	
		
} // namespace


void MainBody()
{
	iVDK::VdkInitDone();
	Utils::DelayInitHost::Inst().DoInit();
	
	BfDev::SysProperties::InitSysFreq118750kHz(); 
			
	// freq info
	{
		int freq = BfDev::SysProperties::Instance().getFrequencyCpu();
		std::cout << "Freq: " << (freq / (1000 * 1000)) << " MHz" << std::endl;		
	}

	// network init
    InitNetwork(true);
    
    BfDev::BfUartTest();
        
    // stop
    {
    	std::cout << "Stop!" << std::endl;
		while(true) iVDK::Sleep(50);	    	
    }
    
    ESS_HALT("Completed!");  
} 

 
