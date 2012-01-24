#ifndef __TCPSERVERSOCKET__
#define __TCPSERVERSOCKET__

#include "Lw/NetUtils.h"
#include "Utils/SafeRef.h"
#include "Utils/HostInf.h"
#include "Utils/QtHelpers.h"
#include "iNet/MsgTcpServer.h"
#include "iNet/ITcpServerToUser.h"
#include "iNet/SocketError.h"
#include "TcpSocket.h"

namespace Lw
{
    using Utils::HostInf;
    using boost::shared_ptr;
    using namespace iNet;

    // TCP server socket
    class TcpServerSocket :
        public virtual Utils::SafeRefServer,
        public ITcpServerToUser,
        boost::noncopyable
    {
        iNet::MsgTcpServer m_srv;

        Platform::Mutex m_mutex;
        std::queue<TcpSocket*> m_incomingSocks;

    // ITcpServerToUser impl
    private:

        void NewConnection(SocketId id, boost::shared_ptr<ITcpSocket> socket)
        {
            Platform::MutexLocker lock(m_mutex);

            m_incomingSocks.push( new TcpSocket(socket) );
        }
        
    public:

        TcpServerSocket() : m_srv(SocksThread(), this)
        {
        }

        ~TcpServerSocket()
        {
            while(TcpSocket *pS = Accept())
            {
                delete pS;
            }
        }

        void Listen(const HostInf &addr)
        {
            m_srv.Listen(addr);
        }

        TcpSocket* Accept()
        {
            TcpSocket *pS = 0;

            {
                Platform::MutexLocker lock(m_mutex);

                if (!m_incomingSocks.empty()) 
                {
                    pS = m_incomingSocks.front();
                    m_incomingSocks.pop();
                }
            }

            return pS;
        }

    };


}  // namespace Lw

#endif

