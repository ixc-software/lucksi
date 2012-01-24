#include "stdafx.h"

#include "TcpSocket.h"

namespace Lw
{
       
    TcpSocket::TcpSocket( int handle, const sockaddr_in &addr ) : m_handle(handle, true)
    {
        m_handle.SetNonBlockMode();
        m_connected = true;

        SockaddrToHostInf(addr, m_remoteSide);
    }

    // ------------------------------------------------

    TcpSocket::TcpSocket() : m_handle(SOCK_STREAM)
    {
        m_handle.SetNonBlockMode();
        m_connected = false;
    }

    // ------------------------------------------------

    void TcpSocket::ConnectStatusUpdate()
    {
        if (!m_connected) return;

        int val;

        // dummi send 0 bytes for detection
        int res = lwip_send(m_handle.get(), &val, 0, 0);

        if (res != 0) 
        {
            m_connected = false;
        }
    }

    // ------------------------------------------------

    bool TcpSocket::Connect(const HostInf &addr)
    {
        ESS_ASSERT(!m_connected);

        sockaddr_in srv;
        HostInfToSockaddr(addr, srv);
        
        int res = lwip_connect(m_handle.get(), (struct sockaddr*)&srv, sizeof(srv));

        m_connected = (res != -1);

        return m_connected;
    }

    // ------------------------------------------------

    bool TcpSocket::Send( const void *p, int count )
    {
        if (!m_connected) return false;

        ESS_ASSERT(count > 0);

        // const_cast 'couse async socket.h header in include :-/
        int res = lwip_send(m_handle.get(), const_cast<void*>(p), count, 0);

        if (res != count) m_connected = false; // ?

        return (res == count);
    }

    // ------------------------------------------------

    bool TcpSocket::Recv( Packet &pack )
    {
        if (!m_connected) return false;

        if (pack.size() < CMinRecvBlockSize) pack.resize(CMinRecvBlockSize);

        int count = Recv(PacketDataBegin(pack), pack.size());
        if (count <= 0) return false;

        pack.resize(count);
        return true;
    }

    // ------------------------------------------------

    int TcpSocket::Recv( void *p, int buffSize )
    {
        ConnectStatusUpdate();

        if (!m_connected) return false;

        unsigned int flags = MSG_DONTWAIT; // non-blocked recv

        int count = lwip_recv(m_handle.get(), p, buffSize, flags);

        if (count >= 0) return count;

        // it's don't work as disconnect detection
        int errCode = GetSocketError();            
        if ((errCode != 0) && (errCode != EAGAIN))
        {
            if (errCode == ENOTCONN) m_connected = false;            	
        }

        return 0;
    }


}  // namespace Lw
