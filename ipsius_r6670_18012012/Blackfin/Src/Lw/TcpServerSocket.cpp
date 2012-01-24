#include "stdafx.h"

#include "TcpServerSocket.h"

namespace Lw
{
           
    TcpServerSocket::TcpServerSocket() : m_handle(SOCK_STREAM)
    {
        m_handle.SetRecvTimeout(1);
    }

    void TcpServerSocket::Listen(const HostInf &addr)
    {
        ESS_ASSERT(m_handle.Bind(addr));

        ESS_ASSERT(lwip_listen(m_handle.get(), CMaxConnections) == 0 && 
        	"Can't listen");
    }

    TcpSocket* TcpServerSocket::Accept()
    {
        sockaddr_in addr;
        int len = sizeof(sockaddr);

        int h = lwip_accept(m_handle.get(), (struct sockaddr*)&addr, &len);

        if (h == -1) return 0;

        return new TcpSocket(h, addr);
    }
    
	Utils::HostInf TcpServerSocket::LocalHostInf() const
	{
		return m_handle.LocalHostInf();
	}
    

}  // namespace Lw

