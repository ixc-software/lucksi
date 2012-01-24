#include "stdafx.h"
#include "MngLwip.h"
#include "Utils/ErrorsSubsystem.h"
#include "Utils/StringParser.h"
#include "Platform/Platform.h"
#include "iVDK/VdkUtils.h"
#include "lwip_sysboot_threadtype.h"
#include "Utils/IntToString.h"

namespace
{
    enum 
    {
        CEmulMode = false, 
    };

	LwIP::Stack *GPStack = 0;

    // ---------------------------------------

    int StringToIP(const std::string &s)
    {
        Utils::ManagedList<std::string> list;
        Utils::StringParser(s, ".", list, false);

        ESS_ASSERT(list.Size() == 4);

        int ip = 0;

        for(int i = 0; i < list.Size(); ++i)
        {
            int val;
            ESS_ASSERT( Utils::StringToInt(*(list[i]), val) );
            ESS_ASSERT( (val >= 0) && ( val <= 255) );

            int shift = 8 * (3 - i);
            ip |= (val << shift);
        }

        return ip;
    }

    // ---------------------------------------

    net_config_info SetConfig(const LwIP::NetworkSettings &settings)
    {
        net_config_info res;

        res.imask = 0;

        res.rx_buffs = 96;
        res.tx_buffs = 96;
        res.rx_buff_datalen = 1024;
        res.tx_buff_datalen = 1024;

        res.buff_area = 0;
        res.buff_area_size = 0;

        res.use_dhcp = settings.DHCP() ? 1 : 0;

        ESS_ASSERT( Lw_GetMAC((char*)&res.mac_addr, sizeof(res.mac_addr)) );

        res.ipaddr      = StringToIP( settings.IP() );
        res.netmask     = StringToIP( settings.Mask() );
        res.gateway     = StringToIP( settings.Gateway() );

        return res;
    }

}


// -----------------------------------

namespace LwIP
{
	
	Stack::Stack(const NetworkSettings &settings)
	{
		ADI_DEV_DEVICE_HANDLE lanHandle;

        net_config_info cfg = SetConfig(settings);
        		
		if ( !Lw_Init(&cfg, &lanHandle) ) ESS_HALT("Lw_Init");

        if (!CEmulMode)
        {
            if ( !Lw_StartMac(lanHandle) ) ESS_HALT("Lw_StartMac");
        }

        if (!Lw_StartStack()) ESS_HALT("Lw_StartStack");

        m_lanHandle = lanHandle;
	}

    // -----------------------------------

    bool Stack::IsEstablished()
    {
        if (CEmulMode)
        {
            return true;
        }

        return Lw_Established(m_lanHandle);
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

    std::string Stack::GetIP()
    {
		char buff[32];

		if ( Lw_GetHostAddr(buff, sizeof(buff)) )
		{
			std::string ip(&buff[0]);
			return ip;
		}

        return "";  // assert - ?
    }
    
	// -----------------------------------------------

	bool Stack::HwEmulationMode()
	{
		return CEmulMode;
	}

	// -----------------------------------------------
	
	void Stack::Init(const NetworkSettings &settings)  // static 
	{
		ESS_ASSERT(GPStack == 0);
		GPStack = new Stack(settings);
	}


	Stack& Stack::Instance()   // static
	{
		ESS_ASSERT(GPStack != 0);		
		return *GPStack;
	}
	
	
}  // namespace LwIP
