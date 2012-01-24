#ifndef __UDPSOCKET__
#define __UDPSOCKET__

#include "Platform/Platform.h"
#include "iNet/MsgUdpSocket.h"
#include "iNet/IUdpSocketToUser.h"
#include "iNet/SocketError.h"
#include "Utils/QtHelpers.h"

#include "NetUtils.h"

namespace Lw
{
    using namespace iNet;
    using boost::shared_ptr;
    using Utils::HostInf;

    /*
        UDP сокет
        Объект создается и используется в "неизвестном" внешнем триде, поэтому
        в некоторых местах нужна синхронизация, т.к. события происходят в 
        контексте SocksThread.
    */
    class UdpSocket : 
        public iNet::IUdpSocketToUser,
        public virtual Utils::SafeRefServer,
        boost::noncopyable
	{
        enum
        {
            CMaxUdpPackSize = 2 * 1024,
            CMaxQueuedPacks = 128,
        };

        iNet::MsgUdpSocket m_sock;

        HostInf m_dst;

        Platform::Mutex m_mutex;
        std::queue< shared_ptr<SocketData> > m_recvData;

        /*
        static boost::shared_ptr<SocketData> DataToSend(const HostAddr &host, int port, 
            void *pData, int dataSize)
        {
            Utils::HostInf hi(host.get(), port);
            SocketData *p = new SocketData(hi, QByteArray((char*)pData, dataSize)); 
            return boost::shared_ptr<SocketData>(p);
        } */

    // IUdpSocketToUser impl
    private:

        void ReceiveData(shared_ptr<SocketData> data)
        {
            Platform::MutexLocker lock(m_mutex);

            if (CMaxQueuedPacks > 0) ESS_ASSERT( m_recvData.size() < CMaxQueuedPacks);

            m_recvData.push(data);
        }

        void SocketErrorOccur(boost::shared_ptr<SocketError> error)
        {
            ESS_HALT( Utils::QStringToString(error->getErrorString()) );
        }

	public:

        UdpSocket(int port) : m_sock(SocksThread(), this, port)
        {
        }

        UdpSocket() : m_sock(SocksThread(), this)
        {
        }
		        
        // return true if bind complete or false else
        bool Bind(int port)
        {
            return m_sock.Bind(port);
        }

        bool SendTo(const HostInf &addr, void *pData, int dataSize, 
            std::string *pErrDesc = 0)
        {
            ESS_ASSERT(addr.IsValid());

            SocketData *p = new SocketData(addr, QByteArray((char*)pData, dataSize));             
            m_sock.SendData( boost::shared_ptr<SocketData>(p) );

            return true;
        }

        bool SendTo(const HostInf &addr, const Packet &data)
        {
            return SendTo(addr, PacketDataBegin(data), data.size());
        }

        void SetDestination(const HostInf &addr)
        {
            ESS_ASSERT(addr.IsValid());

            m_dst = addr;
        }

        bool Send(const Packet &data)
        {
            return Send( PacketDataBegin(data), data.size() );
        }

		bool Send(void *pData, int dataSize)
        {
            return SendTo(m_dst, pData, dataSize);
        }

        bool Recv(Packet &pack, HostInf &addr, bool *pPacketCut = 0)
        {
            pack.resize(CMaxUdpPackSize);

            int count = Recv(PacketDataBegin(pack), pack.size(), addr, pPacketCut);
            if (count <= 0) return false;

            pack.resize(count);
            return true;
        }

        int Recv(void *pData, size_t size, HostInf &addr, bool *pPacketCut = 0)
        {
            shared_ptr<SocketData> data;

            {
                Platform::MutexLocker lock(m_mutex);

                if (!m_recvData.empty()) 
                {
                     data = m_recvData.front();
                     m_recvData.pop();
                }
            }

            if (data.get() == 0) return 0;

            // move data
            QByteArray d = data->getData();
            ESS_ASSERT(size >= d.size());
            std::memcpy(pData, d.constData(), d.size());

            addr = data->getHostinf();

            return d.size();
        }
        
        static int MaxUpdPackSize() { return CMaxUdpPackSize; }
	};

}  // namespace Lw

#endif


