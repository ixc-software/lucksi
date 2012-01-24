#include "stdafx.h"

#include "Utils/ErrorsSubsystem.h"
#include "BfBootSrv/LaunchBooter.h"
#include "BfDev/SysProperties.h"
#include "iUart/BfUart.h"
//#include "BfDev/BfUartProfile.h"


#include "E1App/MngLwip.h"

// flash
#include "BfBootSrv/Flash_M25P128.h"

#include "BfBootSrv/ConfigSetup.h"

#include "BfDev/AdiDeviceManager.h"

#include "Utils/DelayInit.h"

// for FlashLed
#include "Utils/TimerTicks.h"
#include "DevIpTdm/BfLed.h"
#include "DevIpTdm/BfKeys.h"

#include "DevIpTdm/DevIpTdmSpiMng.h"

#include "DevIpTdm/BootFromSpi.h"

#include "cdefBF537.h"
#include "BfBootSrv/ApplicationLoader.h"

#include "BfDev/BfUartSimpleCore.h"
#include "E1App/AppPlatformHook.h"

#include "BfBootCore/BootControlProto.h"

//------------------------------------------------------------

namespace
{ 
	
	const bool CStubCfgCtrl = false; // контролировать конфигурацию платы
	const bool CServiceMode = false; 
	const bool CTraceOn = false;
	
	void TuneStartupParams(BfBootSrv::BooterStartupParams& startParam)
	{
		iLogW::LogSettings srvLogSettings;
    
		if (CTraceOn)		
		{
			// Эти параметры бутут использованны если нет настроек в конфиге
	    	srvLogSettings.out().Udp().DstHost = Utils::HostInf("192.168.0.144", 56001);    
	    	srvLogSettings.out().Udp().TraceInd = true;
    
	    	srvLogSettings.CountSyncroToStore(1);
	    	srvLogSettings.TimestampInd(true);  
		}
		
		
		startParam.LogParams = srvLogSettings;
	    startParam.LogPrefix = "BooterOnBoardProbe";    
	    startParam.TraceActive = CTraceOn;// DevIpTdm::BfKeys::Get(0);
	    startParam.ResetKeyHoldingMsec = 5 * 1000;
	    startParam.ListenPort = BfBootCore::CDefaultCbpPort;//1111;
	    startParam.COM = 0;      
	
	}		
	
    // Use for debug
    /*
	void ReloadStub()
	{
		Platform::dword SpiBaud = DevIpTdm::GetSpiBaud(
	   			DevIpTdm::SpiBusMng::Instance().getSpeedHz()
	   			);   		   			
   	
	    DevIpTdm::BootFromSpi(0, SpiBaud); 
	}
	*/
			


    // Use for debug
	void CfgControl(BfBootSrv::RangeIOBase& flash)
	{
		std::cout << "ConfigSetup..." << std::endl;	
		
		if (CServiceMode)
	    {
	    	BfBootSrv::DeleteDefault(flash);
	    }
	    else
	    {
	    	std::string mac = "90:e0:22:fe:55:80";
	    	BfBootCore::DefaultParam boardPreset(0, 0, mac);
	    	
	    	//boardPreset.OptionalSettings.Network = E1App::NetworkSettings("192.168.0.142", "192.168.0.35", "255.255.255.0");
	    	boardPreset.OptionalSettings.Network = E1App::BoardAddresSettings();	    	
	    	boardPreset.OptionalSettings.Network.get().UseDHCP(mac);
	    	
	    	
	    	boardPreset.OptionalSettings.UserPwd = "DefaultTestPwd";
	    	boardPreset.OptionalSettings.CmpPort = 0;    	    
    
	    	BfBootSrv::ConfigSetup(flash, boardPreset, false);
	    }
	}
   


	void FlashLed(Platform::byte ledNum, Platform::dword periodMsec)
	{
		ESS_ASSERT(ledNum < 2);
		ESS_ASSERT(periodMsec <= 60 * 1000);
		bool leadOn = true;
	    for (int i = 0; i < 5; ++i)
	    {    
	        Platform::ThreadSleep(periodMsec);
	        DevIpTdm::BfLed::Set(ledNum, leadOn); 
	        leadOn = !leadOn;
	    }
	}


	
	void StartApp()
	{
		iVDK::VdkInitDone();
	    BfDev::SysProperties::InitSysFreq133000kHz();        
	    Utils::DelayInitHost::Inst().DoInit();	
		DevIpTdm::DevIpTdmSpiMng spiMng;
	
		Platform::dword spiOffs = 7077888;                
	   	Platform::dword SpiBaud = BfDev::GetSpiBaud(spiMng.GetBfSpi().GetSpeedHz());  
	   	
	   	DevIpTdm::BootFromSpi(spiOffs, SpiBaud);
	}
	

	class AppErrorHook : public ESS::ExceptionHook
	{
	    volatile int m_counter;
	    void Hook(const ESS::BaseException *pE)  // override
	    {
	        ++m_counter;  // for breakpoint 
	        if (dynamic_cast<const ESS::Assertion*>(pE))
	        {
	         	++m_counter;  // for breakpoint             	
	        }
	    }

	public:

	    AppErrorHook() : m_counter(0)
	    {
	    }

	};
	
}

// ------------------------------------------------------------

void MainBody()
{		
	DevIpTdm::BfLed::SetColor(DevIpTdm::GREEN);			  	
	
	//StartApp(); 
	
	iVDK::VdkInitDone();
    BfDev::SysProperties::InitSysFreq133000kHz();
        
    Utils::DelayInitHost::Inst().DoInit();    
    
    E1App::AppPlatformHookSetup(true, true, false); // UART + no reboot
    
    AppErrorHook hook;  // ESS hook              				        

			
	// freq info
	{
		int freq = BfDev::SysProperties::Instance().getFrequencyCpu();
		std::cout << "Freq: " << (freq / (1000 * 1000)) << " MHz" << std::endl;		
	}
	
	//ReloadStub();	
		
	DevIpTdm::DevIpTdmSpiMng spiMng;    
    BfBootSrv::Flash_M25P128 flash( spiMng.CreatePoint( spiMng.GetFlashBusLock() ) );		
    
    BfBootSrv::ApplicationLoader loader( spiMng.CreatePoint( spiMng.GetEchoLoaderBusLock() ) );
    BfBootSrv::BooterStartupParams startParam(flash.getRangeIO(), loader);             
    
    TuneStartupParams(startParam);        
    
      
                       
    if (CStubCfgCtrl) CfgControl(flash.getRangeIO()); // формирование дефолтных предустановок на флеши       
    
    //ESS_HALT("ProbeHalt");	
    
    std::cout << "Launch booter..." << std::endl;
    //DevIpTdm::BfLed::SetColor(DevIpTdm::OFF);		
    BfBootSrv::LaunchBooter(startParam);        
                                             
        
    // stop
    {
    	std::cout << "Stop!" << std::endl;		
    }
    
    ESS_HALT("Completed!"); 
}

