#ifndef __MNGLWIP__
#define __MNGLWIP__

namespace LwIP
{
    using std::string;

    class NetworkSettings
    {
        bool m_useDHCP;

        string m_ip;
        string m_gateway;
        string m_mask;

    public:

        NetworkSettings()
        {
            UseDHCP();
        }

        NetworkSettings(const string &ip, const string &gateway, const string &mask = "255.255.255.0")           
        { 
            Set(ip, gateway, mask);
        }

        void UseDHCP()
        {
            m_useDHCP = true;
        }

        void Set(const string &ip, const string &gateway, const string &mask = "255.255.255.0")
        {
            m_useDHCP = false; 

            m_ip = ip; 
            m_gateway = gateway; 
            m_mask = mask;
        }

        bool DHCP() const { return m_useDHCP; }

        const string& IP() const { return m_ip; }
        const string& Gateway() const { return m_gateway; }
        const string& Mask() const { return m_mask; }
        
    };
	
	class Stack
	{
        void *m_lanHandle;

		Stack(const NetworkSettings &settings);  // init
		
	public:
		
		static void Init(const NetworkSettings &settings);
		static Stack& Instance();
		static bool HwEmulationMode();
		
        bool IsEstablished();
        bool Establish(int timeout = 0);
        std::string GetIP();
        
	};
	
	
}  // namespace LwIP


#endif
