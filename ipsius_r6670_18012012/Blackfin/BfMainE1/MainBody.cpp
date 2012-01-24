
#include "stdafx.h"

#include "BuildInfo.h" 

#include "Utils/ErrorsSubsystem.h"
#include "Utils/DelayInit.h" 

#include "BfDev/SysProperties.h"
#include "BfDev/VdkThreadDcb.h"
#include "BfDev/BfTimerCounter.h"
#include "BfDev/BfTimerCounter.h"

#include "BfBootSrv/Flash_M25P128.h"
#include "BfBootSrv/BoardSetup.h"
#include "BfBootsrv/ConfigSetup.h"
#include "BfBootSrv/ApplicationLoader.h"
#include "BfBootsrv/IReload.h"

#include "SockTest/BfBody.h"

#include "SafeBiProto/SafeBiProto.h"
#include "SafeBiProto/SafeBiProtoTests/SbpSendPackBench.h"

#include "PlatformTests/RecursiveMutexTest.h"
#include "UtilsTests/CPUUsageThreadTest.h"
#include "Ds2155/HdlcTest.h"
#include "DevIpTdm/BfLed.h"
#include "iVDK/VdkCheckInit.h"
#include "iVDK/Mutex.h"
#include "Lw/UdpSocket.h"
#include "iNet/UdpPing.h"
#include "Utils/TimerTicks.h"
#include "EchoBin/HWEchoBin.h"
#include "E1App/MngLwip.h"
#include "E1App/E1AppRunner.h"
#include "E1App/E1AppConfig.h"
#include "E1App/AppCpuUsage.h"
// #include "E1App/AppExceptionHook.h"
#include "E1App/AppPlatformHook.h"

#include "DrvAoz/AozExec.h"
#include "DevIpTdm/AozInit.h"

#include "ChipSecure/MakeSecureResult.h"
#include "ChipSecure/ChipVerify.h"

#include "AllTestsRun.h"
#include "TdmMngTest.h"

namespace
{
    enum
    {
        CStubCfgCtrl                = false, // Контролировать конфигурацию платы
        CSaveEchoBinToSpi           = false,

        CRunAozTest                 = false,  // debug mode
		CRunUdpHugeDataTest    		= false,  // debug mode		
        CSpiSpeedKHz                = 30000,
        
        CFlashLedForever			= false,   // debug mode
        CFlashLedWithEth			= false,   // debug mode used only if CFlashLeadWithEth
    };

            
    // Конфигурация платы. Используется если CStubCfgCtrl.
    const bool CUseDHCP = true;
    const char *CNetMacAddr("02-80-48-32-CA-1B");
    const char *CNetIpAddr("192.168.0.49"); 
    const char *CNetGateway("192.168.0.35");        
    const char *CNetMask("255.255.255.0");      

    const int CDefHwNumber = 1;
    const int CDefHwType = 1;
    const int CDefCmpPort = 44044;  
};
    
// ------------------------------------------------------------

namespace
{
	
	void SetLeds(bool first, bool second)
	{
		DevIpTdm::BfLed::Set(0, first); 
	    DevIpTdm::BfLed::Set(1, second);
	}
	
	void FlashLedForever()
	{
		int i = 0;
		while(true)
		{	
			int state = i++ % 4;
			if (state == 0) SetLeds(1, 0);
			if (state == 1) SetLeds(1, 1);
			if (state == 2) SetLeds(0, 1);
			if (state == 3) SetLeds(1, 1);						
			
			iVDK::Sleep(200);								
		}		
	}
       
    void DefaultNetworkSettings(E1App::NetworkSettings &networkSettings)
    {
        if(CUseDHCP) 
        {
            networkSettings.AddresCfg.UseDHCP(CNetMacAddr);
            return;
        }
        networkSettings.AddresCfg.Set(CNetIpAddr, CNetGateway, 
            CNetMask,CNetMacAddr);
        
    }
    
    
    // --------------------------------------------------------

    void SaveEchoBinToSpi()
    {    	
        if(!CSaveEchoBinToSpi) return;
        
        DevIpTdm::DevIpTdmSpiMng spiMng;
        EchoBin::EchoBinData data = EchoBin::GetEchoBinData();
        BfBootSrv::ApplicationLoader(spiMng.CreatePoint( spiMng.GetEchoLoaderBusLock() )).LoadToSpi(data.Size, data.Data);        
    }
        
    // --------------------------------------------------------

    void NetworkLoop()  // forever
    {
        using E1App::Stack;
        
        bool state = Stack::Instance().IsEstablished();

        while(true)
        {
            bool curr = Stack::Instance().IsEstablished();
            if (curr != state)
            {
                if (curr) std::cout  << "On!"  << std::endl;
                     else std::cout  << "Off!" << std::endl;

                state = curr;
            }

            iVDK::Sleep(50);
        }
    }

    /*
    void NetworkTest()  // forever
    {
        SockTest::RunBfBody();
    } */
    
    void SocksErrorsTest()
    {
        while (true) new Lw::UdpSocket();
    }

    
    void UdpPingTest(int localPort, const Utils::HostInf &dstHost, int timeoutTest, int timeoutBroadcast)
    {
        Lw::UdpSocket socket;

        ESS_ASSERT(socket.Bind(localPort));
        Utils::SafeRef<Lw::UdpSocket> socketRef(&socket);
        iNet::UdpPing udpPing(socketRef);
        std::vector<Platform::byte> data;
        data.push_back(1);
        data.push_back(2);      
        data.push_back(3);
        data.push_back(4);
        data.push_back(5);                      
        
        Utils::TimerTicks timer;
        timer.Set(timeoutTest, false);
        udpPing.Start(dstHost, data, timeoutBroadcast);     
        while (!timer.TimeOut()) 
        {
            udpPing.Process();              
        }
    }
    
    
    void UdpHugeDataTest(int localPort, int timeoutTest, int poolPeriod)
    {
        Lw::UdpSocket socket;

        ESS_ASSERT(socket.Bind(localPort));
        
        
		Utils::TimerTicks periodTimer;
		periodTimer.Set(poolPeriod, true);
		Lw::Packet packet;
		Utils::HostInf host;
		bool pPacketCut = false;
		if(!timeoutTest)
		{
			while (true)
			{
				if(!periodTimer.TimeOut()) continue;

				 socket.Recv(packet, host, &pPacketCut);
			}
			return;
		}
        
		Utils::TimerTicks testTimer;
        testTimer.Set(timeoutTest, false);

		while (!testTimer.TimeOut())
		{
			if(!periodTimer.TimeOut()) continue;

			 socket.Recv(packet, host, &pPacketCut);
		}
      
        std::cout << "Udp huge data test completed" << std::endl;
    }
    
    // --------------------------------------------------------

    extern "C"
    {
        int lwip_GetBuildVersion();
    }
    
    void InitNetwork(E1App::NetworkSettings &settings, bool logOn)
    {
        using E1App::Stack;
        using namespace std;

        int heap = Platform::GetHeapState().BytesFree;

        Stack::Init(settings);
        Stack::Instance().Establish();

        if (logOn)
        {
            cout << "IP: " << Stack::Instance().GetIP() << endl;       

            if (Stack::HwEmulationMode()) cout << "Network emulation!" << endl;

            heap = heap - Platform::GetHeapState().BytesFree;
            // cout << "Lw heap alloc: " <<  heap << endl; 
        }
        
        ESS_ASSERT(lwip_GetBuildVersion() >= 2);
        // cout << "Max socks " << lwip_GetMaxSocks() << endl;           
    }

    // ------------------------------------------------------------
    
	void RunAozExec()
	{
	    DrvAoz::AozExecConfig cfg;
	    cfg.CpuUsage = true;

	    DrvAoz::RunAoz(cfg);
	}
            
    // ------------------------------------------------------------
    
    class CloseE1AppImpl :
        public E1App::ICloseApp
    {          
    	BfBootSrv::IReload& m_reloader;
    	
        void Reboot(Utils::AtomicBool&)
        {
            //breakFlag.Set(true); // pc impl
            m_reloader.Reboot();
        }        
        
     public:
        CloseE1AppImpl(BfBootSrv::IReload& reloader) : m_reloader(reloader) {}
    };  
    
    // ------------------------------------------------------------
    
    struct FlashData
    {
    	BfBootCore::BroadcastMsg Msg;
    	Platform::word CmpPort;
    	E1App::NetworkSettings Network;
    	
    	FlashData()
    	{
    		DevIpTdm::DevIpTdmSpiMng spiMng;
    		BfBootSrv::Flash_M25P128 flash( spiMng.CreatePoint( spiMng.GetFlashBusLock() ) );
    		const BfBootSrv::BoardSetup boardSetup(flash.getRangeIO());
    		
    		
    		if(!boardSetup.ReadNetworkSettings(Network.AddresCfg))  
        	{
            	DefaultNetworkSettings(Network);
        	}  
    		
    		Msg = boardSetup.CreateBroadcastForApp();
    		CmpPort = boardSetup.getCmpPort();     		    		   		
    	}
    };          
    
    
    void StartE1Application()
    {        
        SaveEchoBinToSpi();             
        
        if (CStubCfgCtrl)
        {
        	DevIpTdm::DevIpTdmSpiMng spiMng;
        	BfBootSrv::Flash_M25P128 flash( spiMng.CreatePoint( spiMng.GetFlashBusLock() ) );
        	
            BfBootCore::DefaultParam preset(CDefHwType, CDefHwNumber, CNetMacAddr);     
        
            E1App::NetworkSettings networkPreset;
            DefaultNetworkSettings(networkPreset);
            preset.OptionalSettings.Network = networkPreset.AddresCfg;
            preset.OptionalSettings.CmpPort = CDefCmpPort;      
    
            BfBootSrv::ConfigSetup(flash.getRangeIO(), preset);
        }                        
        
        
        if (CFlashLedForever && !CFlashLedWithEth) FlashLedForever(); // debug
        
        FlashData flashData;
        
        // network         
        InitNetwork(flashData.Network, true);
        
        if (CFlashLedForever && CFlashLedWithEth) FlashLedForever();	// debug
        
        // lock timers for AOZ
        DevIpTdm::AozInit::LockTimers();
        
        // debug mode
        if (CRunAozTest)
        {
        	RunAozExec();
        }
        // debug mode
        if (CRunUdpHugeDataTest)
        {
        	UdpHugeDataTest(19000, 0, 500);
        }
           
        // run                
        E1App::E1AppConfig config;      
        config.AppBuildInfo = AutoBuildInfo::FullInfo(); 
        BfBootSrv::ApplicationLoader reloader;   
        CloseE1AppImpl appClose(reloader);        
        
        E1App::RunE1Application(reloader, appClose, config, flashData.Msg, flashData.CmpPort);
    };    
    
    
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
    

}; // namespace


// iVDK::Mutex GMutex;  // check VDK init 

// ------------------------------------------------------------

void EnterTests()
{
    // ...
    
    // DCB thread test
    // BfDev::VdkThreadDcbTest();
    // UdpPingTest(19000, Utils::HostInf("255.255.255.255", 5062), 60000, 200);
    // socks error codes test
    // SocksErrorsTest();
    
    // CPU usage
    // UtilsTests::Win32CpuUsageTest();

    // TDM test    
    // ExecuteTdmTest();

    // network tests
    // NetworkLoop();
    // NetworkTest();       
    
    // all test run
    // ExecuteAllTests();       
    
    // SBP
    // ExecuteSafeBiProtoTest();
    
    // HDLC tests
    // Ds2155::RunHdlcTests();  
}

// ------------------------------------------------------------

void MainBody()
{
    DevIpTdm::BfLed::SetColor(DevIpTdm::OFF); // DevIpTdm::GREEN
        
    iVDK::VdkInitDone();
    BfDev::SysProperties::InitSysFreq133000kHz();
        
    Utils::DelayInitHost::Inst().DoInit();
    DevIpTdm::BfLed::Set(1, true);  
    
    E1App::AppPlatformHookSetup(true, true, false); // UART + no reboot
    
    AppErrorHook hook;  // ESS hook
    
    std::string cmpVer = AutoBuildInfo::CmpProtoInfo(); // dummi for forced linking
              
    // freq info
    {
        int freq = BfDev::SysProperties::Instance().getFrequencyCpu();
        // std::cout << "Freq: " << (freq / (1000 * 1000)) << " MHz" << std::endl;     
    }
    
    // ChipSecure::MakeSecureDump();
//    ChipSecure::ChipVerify::TestAll();
    StartE1Application();
        
    // stop
    {
        std::cout << "Stop!" << std::endl;
        while(true) iVDK::Sleep(50);          
    }
    
    ESS_HALT("Completed!");  
}

