#ifndef __TCPSOCKET__
#define __TCPSOCKET__

#include "Lw/NetUtils.h"

namespace Lw
{

    class TcpSocket : boost::noncopyable
    {
        SockHandle m_handle;
        bool m_connected;
        HostInf m_remoteSide;

        enum
        {
            CMinRecvBlockSize = 256,
        };

        friend class TcpServerSocket;

        TcpSocket(int handle, const sockaddr_in &addr);
        
        void ConnectStatusUpdate();


    public:

        TcpSocket();

        bool Connect(const HostInf &addr);

        bool Send(Packet &pack)
        {
            return Send( PacketDataBegin(pack), pack.size() );
        }

        bool Send(const void *p, int count);

        bool Recv(Packet &pack);

        int Recv(void *p, int buffSize);

        bool Connected() const 
        {        	
            return m_connected;
        }

        const HostInf& RemoteSide() const { return m_remoteSide; }

    };

}  // namespace Lw

#endif



