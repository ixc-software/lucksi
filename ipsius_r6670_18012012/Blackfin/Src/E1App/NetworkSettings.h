#ifndef _NETWORK_SETTINGS_H_
#define _NETWORK_SETTINGS_H_

#include "Utils/HostInf.h"

namespace E1App
{
    using std::string;

    // Адресные настройки сетевого стека
    class BoardAddresSettings
    {
        bool m_useDHCP;

        string m_ip;
        string m_gateway;
        string m_mask;

        // optional; format "00-80-48-12-AA-1B", six hex bytes with any separator cha
        string m_MAC;  

    public:

        BoardAddresSettings()
        {
            UseDHCP();
        }

        BoardAddresSettings(const string &ip, const string &gateway, 
            const string &mask = "255.255.255.0",
            const string mac = "")           
        { 
            Set(ip, gateway, mask, mac);
        }

        void UseDHCP(const std::string& mac = "")
        {
            m_useDHCP = true;
            m_MAC = mac;
        }

        

        void Set(const string &ip, const string &gateway, 
            const string &mask = "255.255.255.0",
            const string &mac = "")
        {
            m_useDHCP = false; 

            m_ip = ip; 
            m_gateway = gateway; 
            m_mask = mask;

            m_MAC = mac;
        }
		bool IsValid() const
		{
			if (m_useDHCP) return true;

            Platform::dword dummy;
            return Utils::HostInf::StringToIP(m_ip, dummy) &&
				Utils::HostInf::StringToIP(m_gateway, dummy) &&
				Utils::HostInf::StringToIP(m_mask, dummy);
		}

        bool DHCP() const { return m_useDHCP; }

        const string& IP() const { return m_ip; }
        const string& Gateway() const { return m_gateway; }
        const string& Mask() const { return m_mask; }
        const string& MAC() const { return m_MAC; }        
    };

    struct NetworkBuffersSettings
    {      
        int rx_buffs;
        int tx_buffs;
        int rx_buff_datalen;
        int tx_buff_datalen;        

        NetworkBuffersSettings()
        {
            // default
            rx_buffs = 96;
            tx_buffs = 96;
            rx_buff_datalen = 1024 * 2;
            tx_buff_datalen = 1024 * 2;
        }
    };

    struct NetworkSettings 
    {
        NetworkBuffersSettings BuffCfg;
        BoardAddresSettings AddresCfg;
    };
}  // namespace E1App


#endif
