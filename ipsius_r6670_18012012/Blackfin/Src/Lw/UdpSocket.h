#ifndef __UDPSOCKET__
#define __UDPSOCKET__

#include "Utils/SafeRef.h"
#include "Lw/NetUtils.h"

namespace Lw
{

	class UdpSocket : boost::noncopyable,
		public virtual Utils::SafeRefServer
	{
        SockHandle m_handle;

        HostInf m_dst; 

        // stats
        int m_packsSend;
        int m_packsRecv;

        void Init()
        {
            m_packsSend = 0;
            m_packsRecv = 0;

            m_handle.SetNonBlockMode();
        }

        void AssertBind()
        {
            ESS_ASSERT(m_handle.Binded());
        }

	public:

		UdpSocket(int port); // with bind
		UdpSocket();
		
        ~UdpSocket();
        
        // return true if bind complete or false else
        bool Bind(int port);

        bool SendTo(const HostInf &addr, const void *pData, int dataSize, 
            std::string *pErrDesc = 0);
        bool SendTo(const HostInf &addr, const Packet &data);

        void SetDestination(const HostInf &addr);
        bool Send(const Packet &data);
		bool Send(const void *pData, int dataSize);        

        bool Recv(Packet &pack, HostInf &addr, bool *pPacketCut = 0);
        int  Recv(void *pData, size_t size, HostInf &addr, bool *pPacketCut = 0);
        
        static int MaxUpdPackSize();
	};

}  // namespace Lw

#endif


