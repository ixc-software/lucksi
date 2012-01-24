#ifndef __BFSBPSERVER__
#define __BFSBPSERVER__

#include "SafeBiProto/SafeBiProto.h"
#include "ILog.h"
#include "Lw/TcpSocket.h"
#include "Lw/TcpServerSocket.h"

namespace SBProto
{
    using boost::scoped_ptr;    

    class BfSbpServer : public ISbpProtoTransport 
    {
        ISbpTransportToProto *m_pProto;
        ILog &m_log;
    	SbpSettings m_settings;
        SafeBiProto m_proto;

        Lw::TcpServerSocket m_srv;
        scoped_ptr<Lw::TcpSocket> m_sock;

        Lw::Packet m_recvBuff;

        void GoesConnected()
        {
            {
                std::ostringstream os;
                os << "Connected " << m_sock->RemoteSide().ToString();
                m_log.LogSBP(os.str());
            }

            m_proto.Activate();
        }

        void GoesDisconnected()
        {
            if (m_sock.get() == 0) return;

            m_log.LogSBP("Disconnected");

            m_proto.Deactivate();

            m_sock.reset(0);
        }

    // ISbpProtoTransport impl
    private:

        void BindProto(ISbpTransportToProto *pProto)
        {
            m_pProto = pProto;
        }

        void UnbindProto()
        {
            m_pProto = 0;
        }

        void Send(const void *pData, size_t size)
        {
            if (m_sock.get() == 0) return;

            bool ok = m_sock->Send(pData, size);
            if (!ok) GoesDisconnected();
        }

        
    public:

        BfSbpServer(ISafeBiProtoEvents &owner, ISendPackMonitor *pMonitor, ILog &log, int listenPort)
            : m_pProto(0), m_log(log), m_proto(owner, *this, m_settings, pMonitor)
        {
            m_srv.Listen(listenPort);
        }

        void Process()
        {
            // accept block execution for 1 ms 
            // that is why we try accept only if no client already connected
            // normal work -- always accept and drop new connection if has already one
            if (m_sock.get() == 0)
            {
                Lw::TcpSocket *p = m_srv.Accept();
                if (p != 0)
                {
                    m_sock.reset(p);
                    GoesConnected();
                }
            }

			// SBP process
            m_proto.Process();

            // socket poll
            if (m_sock.get() != 0)
            {
                while(m_sock->Recv(m_recvBuff))
                {
                    if (m_pProto != 0)
                    {
                        void *p = Lw::PacketDataBegin(m_recvBuff);
                        m_pProto->DataReceived(p, m_recvBuff.size());
                    }
                }

                if (!m_sock->Connected())
                {
                    GoesDisconnected();
                }
            }
						
        }

        bool Connected() const  { return (m_sock.get() != 0); }

        SafeBiProto& Proto()    { return m_proto; }

        void OnPhConnect()
        {
            // nothing
            // GoesConnected();
        }

        void OnPhDisconnect()
        {
            GoesDisconnected();
        }
                
    };
	
}  // namespace SafeBiProto

#endif
