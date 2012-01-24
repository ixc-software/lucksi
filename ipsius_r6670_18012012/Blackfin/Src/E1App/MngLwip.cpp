#include "stdafx.h"
#include "MngLwip.h"
#include "Utils/ErrorsSubsystem.h"
#include "Utils/StringParser.h"
#include "Utils/IntToString.h"
#include "Platform/Platform.h"
#include "iVDK/VdkUtils.h"
#include "lwip_sysboot_threadtype.h"
#include "Utils/IntToString.h"
#include "Utils/HostInf.h"

namespace
{
    using Platform::dword;
    using Utils::HostInf;
    
    const bool CTraceToConsole = false;
    
    enum 
    {
        CEmulMode = false, 
    };

	E1App::Stack *GPStack = 0;

    // --------------------------------------- 

	dword StringToIP(const std::string &s)
    {
    	dword ip;
    	ESS_ASSERT( HostInf::StringToIP(s, ip) );
    	return ip;
    }
	
    // ---------------------------------------

    net_config_info SetConfig(const E1App::NetworkSettings &settings)
    {
        net_config_info res;

        res.imask = 0;

        res.rx_buffs = settings.BuffCfg.rx_buffs;
        res.tx_buffs = settings.BuffCfg.tx_buffs;
        res.rx_buff_datalen = settings.BuffCfg.rx_buff_datalen;
        res.tx_buff_datalen = settings.BuffCfg.tx_buff_datalen;

        res.buff_area = 0;
        res.buff_area_size = 0;

        res.use_dhcp = settings.AddresCfg.DHCP() ? 1 : 0;

        if (settings.AddresCfg.MAC().empty())
        {
            ESS_ASSERT( Lw_GetMAC((char*)&res.mac_addr, sizeof(res.mac_addr)) );
        }
        else
        {
            ESS_ASSERT( HostInf::StringToMac(settings.AddresCfg.MAC(), res.mac_addr, sizeof(res.mac_addr)) );
        }

        if (!res.use_dhcp)
        {
        	res.ipaddr      = StringToIP( settings.AddresCfg.IP() );
        	res.netmask     = StringToIP( settings.AddresCfg.Mask() );
        	res.gateway     = StringToIP( settings.AddresCfg.Gateway() );
        }
        else
        {
        	res.ipaddr      = 0;
        	res.netmask     = 0;
        	res.gateway     = 0;
        }

        return res;
    }

}


// -----------------------------------

namespace E1App
{
	
    Stack::Stack(const NetworkSettings &settings, bool doStart ) :
        m_MAC( settings.AddresCfg.MAC() )
	{
		if (CTraceToConsole) std::cout << "Stack()..." << std::endl;;
		
		ADI_DEV_DEVICE_HANDLE lanHandle;

        net_config_info cfg = SetConfig(settings);
        		
		if ( !Lw_Init(&cfg, &lanHandle) ) ESS_HALT("Lw_Init");

        if (!CEmulMode)
        {
            if ( !Lw_StartMac(lanHandle) ) ESS_HALT("Lw_StartMac");
        }                

        m_started = false;
        if (doStart) Start();                
       
        m_lanHandle = lanHandle;
        
        if (CTraceToConsole) std::cout << "Stack created." << std::endl;;
	}
	
	Stack::~Stack()
	{
		ESS_ASSERT( Lw_Shutdown(m_lanHandle) );
	}

    // -----------------------------------

    bool Stack::IsEstablished() const 
    {
        return (CEmulMode) ? true : Lw_Established(m_lanHandle);
    }

    // -----------------------------------

    bool Stack::Establish(int timeout)
    {
        using Platform::GetSystemTickCount;

        int startTime = GetSystemTickCount();

        while(true)
        {
            if (IsEstablished()) return true;

            iVDK::Sleep(5);

            if (timeout > 0)
            {
                if (GetSystemTickCount() - startTime > timeout) break;
            }
        }

        return false;
    }

    // -----------------------------------

    std::string Stack::GetIP() const 
    {
		char buff[32];

		if ( Lw_GetHostAddr(buff, sizeof(buff)) )
		{
			return std::string(&buff[0]);
		}

        return "";  // assert - ?
    }
    
	// -----------------------------------------------

	bool Stack::HwEmulationMode()
	{
		return CEmulMode;
	}

	// -----------------------------------------------
	
	void Stack::Init(const NetworkSettings &settings, bool doStart)  // static 
	{
		ESS_ASSERT(GPStack == 0);
		GPStack = new Stack(settings, doStart);
	}
	
	void Stack::Shutdown()
	{
		ESS_ASSERT(GPStack != 0);
		
		delete GPStack;
		GPStack = 0;
	}


    // -----------------------------------------------

	Stack& Stack::Instance()   // static
	{
		ESS_ASSERT(GPStack != 0);		
		return *GPStack;
	}

    // -----------------------------------------------
	
    bool Stack::IsInited()   // static
    {
        return GPStack != 0;
    }
    
    // ------------------------------------------------
    
    void Stack::Start()
    {
      	ESS_ASSERT(!m_started);
      	if (CTraceToConsole) std::cout << "Lw_StartStack..." << std::endl;
        ESS_ASSERT(Lw_StartStack() && "Lw_StartStack");
        m_started = true;
        if (CTraceToConsole) std::cout << "Lw_StartStack complete" << std::endl;
    }
	
}  // namespace E1App
