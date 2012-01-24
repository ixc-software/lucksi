#ifndef __TCPSERVERSOCKET__
#define __TCPSERVERSOCKET__

#include "Lw/NetUtils.h"
#include "TcpSocket.h"

namespace Lw
{

    class TcpServerSocket : boost::noncopyable
    {
        SockHandle m_handle;
        
        enum { CMaxConnections = 32, };

    public:

        TcpServerSocket();

        void Listen(const HostInf &addr);

        // don't use shared_ptr here for more freedom at user side
        // it can block execution during 1-2 ms !!
        TcpSocket* Accept();
        
		Utils::HostInf LocalHostInf() const;
      

    };


}  // namespace Lw

#endif

