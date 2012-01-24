#include "stdafx.h"
#include "SafeBiProto/SafeBiProto.h"
#include "iNet/MsgTcpSocket.h"
#include "iNet/SocketData.h"
#include "iNet/SocketError.h"
#include "SbpTcpTransportFactory.h"

namespace 
{
	using namespace SBProto;

    class SbpTcpTransportImpl : boost::noncopyable, 
        public virtual Utils::SafeRefServer,
        public ISbpTransport,
        public iNet::ITcpSocketToUser
    {
    public:
        SbpTcpTransportImpl(iCore::MsgThread &thread, const Utils::HostInf &host, ISbpTransportToUser *user, size_t maxSendSize) : 
			m_isServerSide(false),
            m_socket(new iNet::MsgTcpSocket(thread, this)), 
            m_user(user), m_proto(0), m_host(host) 
        {
            m_socket->LinkUserToSocket(this);
            if(m_user) Connect();
        }

        SbpTcpTransportImpl(boost::shared_ptr<iNet::ITcpSocket> socket, size_t maxSendSize) : 
			m_isServerSide(true),
            m_socket(socket), m_user(0), m_proto(0), m_maxSendSize(maxSendSize)
        {
            ESS_ASSERT(socket != 0);

            m_socket->LinkUserToSocket(this);
            m_host = socket->PeerHostInf();
        }

        ~SbpTcpTransportImpl()
        {
            // Protocol should be destroyed before transport! 
            ESS_ASSERT(m_proto == 0);
        }

        // -------------------------------------------------------------------
        // iNet::ITcpSocketToUser

        void Connected(iNet::SocketId id)
        {
            ESS_ASSERT(m_user != 0);
            m_user->TransportConnected();
        }

        // -------------------------------------------------------------------

        void ReceiveData(boost::shared_ptr<iNet::SocketData> data)
        {
            ESS_ASSERT(m_proto != 0);

            m_proto->DataReceived(data->getData().data(), data->getData().size());
        }

        // -------------------------------------------------------------------

        void Disconnected(boost::shared_ptr<iNet::SocketError> error)
        {
            ESS_ASSERT(m_user != 0);
            m_user->TransportDisconnected((error == 0) ?
				"Transport disconnected" :
				error->getErrorString().toStdString());
        }

        // -------------------------------------------------------------------
        // ISbpTransport 

        size_t MaxSendSize() const
        {
            return m_maxSendSize;
        }

        void BindProto(ISbpTransportToProto *proto) 
        {
            ESS_ASSERT(m_proto == 0 && proto != 0);
            
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
            ESS_ASSERT(pData != 0);
            ESS_ASSERT(size != 0);            

            Utils::MemReaderStream stream(pData, size);
            Utils::BinaryReader<Utils::MemReaderStream> reader(stream);
            QByteArray sendData;
            sendData.resize(size);
            reader.ReadData(sendData.data(), size);

            m_socket->SendData(sendData);
        }

        // -------------------------------------------------------------------

        void Connect() 
        {
            ESS_ASSERT(m_user != 0);

            if(!m_isServerSide) m_socket->ConnectToHost(m_host); 
        }

        // -------------------------------------------------------------------

        void Disconnect() 
        {
            ESS_ASSERT(m_user != 0);

            m_socket->DisconnectFromHost(); 
        }

        // -------------------------------------------------------------------

        void BindUser(ISbpTransportToUser *pUser)
        {
            ESS_ASSERT(pUser != 0 && m_user == 0);

            m_user = pUser;
        }

        std::string Info() const
        {
            if (m_socket->LocalHostInf().Empty()) return "";
            
            std::ostringstream out;
            out << "Local addr: " << m_socket->LocalHostInf().ToString();
            if(!m_socket->PeerHostInf().Empty())
            {
                out << "; Remote addr: " << m_socket->PeerHostInf().ToString();
            }
            return out.str();
        }

        void Process()
        {}
    private:   
		bool m_isServerSide;
        boost::shared_ptr<iNet::ITcpSocket> m_socket;
        ISbpTransportToUser *m_user;
        ISbpTransportToProto *m_proto;
        Utils::HostInf m_host;
        size_t m_maxSendSize;
    };
};

// -------------------------------------------------------------------

namespace SBProto
{
	boost::shared_ptr<ISbpTransport> SbpTcpTransportFactory::CreateTransport(
		iCore::MsgThread &thread, 
		const Utils::HostInf &host,
		ISbpTransportToUser *user, 
        size_t maxSendSize)
	{
		return boost::shared_ptr<ISbpTransport>(new SbpTcpTransportImpl(thread, 
			host, user, maxSendSize)); 
	}

    // -------------------------------------------------------------------

	boost::shared_ptr<ISbpTransport> SbpTcpTransportFactory::CreateTransport(
		boost::shared_ptr<iNet::ITcpSocket> socket, size_t maxSendSize)
    {
		return boost::shared_ptr<ISbpTransport>(new SbpTcpTransportImpl(socket, maxSendSize)); 
	}

} // namespace SBProto
