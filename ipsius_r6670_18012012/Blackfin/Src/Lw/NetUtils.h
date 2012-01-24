#ifndef __NETUTILS__
#define __NETUTILS__

#include "stdafx.h"

// LwIP
#define LWIP_PROVIDE_ERRNO

#include <lwip/sockets.h>
#include <lwip/inet.h>
#include <lwip/cglobals.h>

#undef LWIP_PROVIDE_ERRNO

#include "Utils/ErrorsSubsystem.h"
#include "Utils/HostInf.h"
#include "Platform/Platform.h"

namespace Lw
{
    using Platform::byte;
    using Utils::HostInf;

    typedef std::vector<byte> Packet;

    static void* PacketDataBegin(const Packet &v)
    {
        ESS_ASSERT(v.size() > 0);
        return (void*)&(v[0]);
    }
    
    static void HostInfToSockaddr(const HostInf &addr, sockaddr_in &result)
    {
		result.sin_family = AF_INET;
		result.sin_addr.s_addr = addr.InetAddr();
		result.sin_port = htons( addr.Port() );           	
    }

    static void SockaddrToHostInf(const sockaddr_in &addr, HostInf &result)
    {
        result.InetAddr(addr.sin_addr.s_addr, true);
        result.Port( ntohs(addr.sin_port) );
    }

    // ---------------------------------------
    // basic socket funtions
    
    class SockHandle : boost::noncopyable
    {
        static int GSockCounter;

        int m_handle;
        bool m_binded;

    public:

        SockHandle(int type) : m_binded(false)
        {
            m_handle = lwip_socket(AF_INET, type, 0);            
            if (m_handle < 0)
            {
            	int errCode = GetSocketError();
                std::ostringstream ss;
                ss << "Sock creation fail, total socks " << GSockCounter
                   << "; code " << errCode;
                ESS_HALT(ss.str());
            }

            GSockCounter++;
        }

        // second param is dummi just for c-tor overload
        SockHandle(int handle, bool createFromHandle) : m_binded(false)
        {
            ESS_ASSERT(handle >= 0);

            m_handle = handle;
            GSockCounter++;
        }

        ~SockHandle()
        {
            lwip_close(m_handle);

            ESS_ASSERT(GSockCounter > 0);
            GSockCounter--;
        }

        int get() const { return m_handle; }

        /*
            Library must be builded with 
            #define LWIP_SO_RCVTIMEO 1
            in lwipopts.h
        */
        void SetRecvTimeout(int value)
        {
            int res = lwip_setsockopt(m_handle, SOL_SOCKET, SO_RCVTIMEO, &value, sizeof(value));
            ESS_ASSERT((res == 0) && "LwIP don't support SO_RCVTIMEO! Use correct library version!");
        }

        void SetNonBlockMode()
        {
            int on = 1;
            int res = lwip_ioctl(m_handle, FIONBIO, &on);
            ESS_ASSERT(res == 0);
        }

        bool Bind(const HostInf &addr)
        {
            if (m_binded)
            {
                ESS_HALT("Trying rebind. Rebind not allowed.");
            }

            sockaddr_in ra;
            HostInfToSockaddr(addr, ra);

            // typecast to sockaddr* is sock api evil
            m_binded = 
                (lwip_bind(m_handle, (sockaddr*)&ra, sizeof(ra)) != -1); 

            return m_binded;            
        }
		
		Utils::HostInf LocalHostInf() const
		{
            sockaddr_in ra;
            int size = sizeof(ra);
            lwip_getsockname (m_handle, (sockaddr*)&ra, &size);// == -1) return Utils::HostInf(); 
			Utils::HostInf result;
            SockaddrToHostInf(ra, result);
            return result;
		}
        
        bool Binded() const { return m_binded; }

    };



}  // namespace Lw

#endif

