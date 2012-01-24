#ifndef __MNGLWIP__
#define __MNGLWIP__

#include "NetworkSettings.h"

namespace E1App
{    

    class Stack : boost::noncopyable
	{
        void *m_lanHandle;
        bool m_started;
        const std::string m_MAC;

		Stack(const NetworkSettings &settings, bool doStart);  // init
		~Stack();
		
	public:
		
		static void Init(const NetworkSettings &settings, bool doStart = true);
		static Stack& Instance();
		static bool HwEmulationMode();
        static bool IsInited();
        static void Shutdown();
       		
        bool IsEstablished() const;
        bool Establish(int timeout = 0);
        bool Started() const {return m_started;}
        void Start();

        std::string GetIP() const;
        std::string MAC() const { return m_MAC; }


        bool IpIsEmpty() const
        {
            Platform::dword ip;
            ESS_ASSERT( Utils::HostInf::StringToIP(GetIP(), ip) );
            return ip == 0;
        }
        
	};
	
	
}  // namespace E1App


#endif
