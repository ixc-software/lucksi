#include "stdafx.h"
#include "Lw/TcpSocket.h"
#include "SbpTcpTransportFactory.h"

namespace 
{
	using namespace SBProto;
	
    class SbpTcpTransportImpl : boost::noncopyable, 
        public ISbpTransport
    {
    public:
        SbpTcpTransportImpl(Lw::TcpSocket *socket, size_t maxSendSize) : 
            m_socket(socket), m_user(0), m_proto(0)
        {
        	m_maxSendSize = maxSendSize;
			m_connected = false;
            ESS_ASSERT(socket != 0);
        }

        ~SbpTcpTransportImpl()
        {
            // Protocol should be destroyed before transport! 
            ESS_ASSERT(m_proto == 0);
        }


    // ISbpTransport 
    public:
    
    	size_t MaxSendSize()const
    	{
    		return m_maxSendSize;
    	}
    
        void BindProto(ISbpTransportToProto *proto) 
        {
            ESS_ASSERT(!m_proto && proto != 0);
            
            m_proto = proto;
        }

        // -------------------------------------------------------------------

        void UnbindProto()
        {
            ESS_ASSERT(m_proto != 0);
            
            m_proto = 0;
        }

        // -------------------------------------------------------------------

        void Send(const void *pData, size_t size)
        {
            if(m_socket == 0) return;

            bool ok = m_socket->Send(pData, size);
            if (!ok) Disconnected();
        }

        // -------------------------------------------------------------------

        void Connect() 
        {
            ESS_ASSERT(m_user != 0);
        }

        // -------------------------------------------------------------------

        void Disconnect() 
        { 
            ESS_ASSERT(m_user != 0);
        }

        // -------------------------------------------------------------------

        void BindUser(ISbpTransportToUser *pUser)
        {
            ESS_ASSERT(pUser != 0 && m_user == 0);
            
            m_user = pUser;
        } 

        // -------------------------------------------------------------------

        std::string Info() const
        {
        	return  m_socket->RemoteSide().ToString();
        }

        // -------------------------------------------------------------------

        void Process()
        {
            if(m_socket == 0) return;

            while(m_socket->Recv(m_recvBuff))
            {
                if (m_proto != 0)
                {
                    void *p = Lw::PacketDataBegin(m_recvBuff);
                    m_proto->DataReceived(p, m_recvBuff.size());
                }
            }
            if (m_socket->Connected()) 
            	Connected();
            else
            	Disconnected();
        }
        
    private:   
        void Connected()
       	{
			if(m_connected)  return;
			m_connected = true;
            ESS_ASSERT(m_user != 0);
            m_user->TransportConnected();
       	
       	}
        void Disconnected()
        {
			if(!m_connected)  return;
			m_connected = false;
			
            ESS_ASSERT(m_user != 0);
            m_user->TransportDisconnected("Transport disconnected");
        }

    private:   
        boost::scoped_ptr<Lw::TcpSocket> m_socket;
        bool m_connected;
        Lw::Packet m_recvBuff;
        ISbpTransportToUser *m_user;
        ISbpTransportToProto *m_proto;
        size_t m_maxSendSize;
    };
};

namespace SBProto
{
 
	boost::shared_ptr<ISbpTransport>  
    	SbpTcpTransportFactory::CreateTransport(Lw::TcpSocket *socket, size_t maxSendSize)  
    {
		return boost::shared_ptr<ISbpTransport>(new SbpTcpTransportImpl(socket, maxSendSize)); 
    }

    // -------------------------------------------------------------------

	boost::shared_ptr<ISbpTransport>  
    	SbpTcpTransportFactory::CreateTransport(iCore::MsgThread &thread, 
                                     const Utils::HostInf &host,
                                     ISbpTransportToUser *user, size_t maxSendSize)  
    {
		ESS_HALT("Not allowed on Blackfin");    	
		return boost::shared_ptr<ISbpTransport>(); 
    }

} // namespace SBProto

