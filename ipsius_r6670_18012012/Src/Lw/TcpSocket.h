#ifndef __TCPSOCKET__
#define __TCPSOCKET__

#include "iNet/ITcpSocket.h"
#include "iNet/ITcpSocketToUser.h"
#include "iNet/MsgTcpSocket.h"
#include "iNet/SocketError.h"
#include "Utils/SafeRef.h"
#include "Utils/QtHelpers.h"
#include "Lw/NetUtils.h"

namespace Lw
{
    using namespace iNet;
    using boost::shared_ptr;
    using Utils::HostInf;

    class TcpSocket :
        public virtual Utils::SafeRefServer,
        public ITcpSocketToUser,
        boost::noncopyable
    {
        enum
        {
            CTcpPackSize = 1024,
        };

        shared_ptr<ITcpSocket> m_sock;
        bool m_connected;

        Platform::Mutex m_mutex;
        std::queue<QByteArray> m_recvData;

        friend class TcpServerSocket;
        
        TcpSocket(shared_ptr<ITcpSocket> sock)
        {
            m_sock = sock;
            m_sock->LinkUserToSocket(this);

            m_connected = true;
        }

    // ITcpSocketToUser impl
    private:

        void Connected(SocketId id)
        {
            m_connected = true;
        }

        void ReceiveData(shared_ptr<SocketData> data)
        {
            Platform::MutexLocker lock(m_mutex);

            m_recvData.push( data->getData() );
        }

        void Disconnected(shared_ptr<SocketError> error)
        {
            m_connected = false;
        }

        void SocketErrorOccur(shared_ptr<SocketError> error)
        {
            ESS_HALT( Utils::QStringToString(error->getErrorString()) );
        }


    public:

        TcpSocket()
        {
            m_sock.reset( new MsgTcpSocket(SocksThread(), this) );

            m_connected = false;
        }

        bool Connect(const HostInf &addr)
        {
            m_sock->ConnectToHost(addr);
        }

        bool Send(Packet &pack)
        {
            return Send( PacketDataBegin(pack), pack.size() );
        }

        bool Send(const void *p, int count)
        {
            QByteArray data((char*)p, count);
            m_sock->SendData(data);

            return true;
        }

        bool Recv(Packet &pack)
        {
            pack.resize(CTcpPackSize);

            int count = Recv(PacketDataBegin(pack), pack.size());
            if (count <= 0) return false;

            pack.resize(count);
            return count;
        }

        int Recv(void *p, int buffSize)
        {
            QByteArray data;

            {
                Platform::MutexLocker lock(m_mutex);

                if (!m_recvData.empty()) 
                {
                    data = m_recvData.front();
                    m_recvData.pop();
                }
            }

            if (data.size() == 0) return 0;

            // move data
            ESS_ASSERT(buffSize >= data.size());
            std::memcpy(p, data.constData(), data.size());            

            return data.size();
        }

        bool Connected() const 
        {        	
            return m_connected;
        }

        const HostInf& RemoteSide() const 
        { 
            return m_sock->PeerHostInf(); 
        }

    };

}  // namespace Lw

#endif



