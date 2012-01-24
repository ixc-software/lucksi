
#ifndef __TESTTCPTRANSPORTS__
#define __TESTTCPTRANSPORTS__

// TestTcpTransports.h

#include "TestUtils/TestTcpServerSocket.h"
#include "TestUtils/TestTcpSocket.h"
#include "SafeBiProto/ISbpTransport.h"
#include "SafeBiProto/ISbpUserTransportServer.h"

namespace SBPTests
{
    using namespace SBProto;
    using namespace TestUtils;

    // ------------------------------------------------------------------
    
    // Base class for Tcp transports
    class TestTcpTransport:
        public virtual Utils::SafeRefServer,
        public ISbpTransport,
        public ITcpSocketEmulToUser,
        public boost::noncopyable
    {
        boost::shared_ptr<ITcpSocketEmul> m_socket;
        ISbpTransportToUser *m_pUser;
        ISbpTransportToProto *m_pProto;
        Utils::HostInf m_host;

    // iNet::ITcpSocketToUser impl
    private:
        void Connected()
        {
            TUT_ASSERT(m_pUser != 0);
            m_pUser->TransportConnected();
        }
        
        void ReceiveData(boost::shared_ptr<Utils::ManagedMemBlock> data)
        {
            TUT_ASSERT(m_pProto != 0);
            m_pProto->DataReceived(data->getData(), data->Size());
        }

        void Disconnected(boost::shared_ptr<SocketError> desc)
        {
            TUT_ASSERT(m_pUser != 0);
			m_pUser->TransportDisconnected((desc == 0) ? 
				"Transport disconnected"  : desc->ToString());
        }

    // ISbpTransport impl
    private:
        void BindProto(ISbpTransportToProto *pProto)
        {
            TUT_ASSERT(pProto != 0);
            m_pProto = pProto;
        } 
        
        void UnbindProto()
        {
            TUT_ASSERT(m_pProto != 0);
            m_pProto = 0;
        }
        
        void Send(const void *pData, size_t size)
        {
            TUT_ASSERT(pData != 0);
            TUT_ASSERT(size != 0);
            TUT_ASSERT(m_socket.get() != 0);
            m_socket->SendData(pData, size);
        }

        void Connect() 
        {
            TUT_ASSERT(m_socket.get() != 0);
            m_socket->ConnectToHost(m_host); 
        }
        
        void Disconnect() 
        { 
            TUT_ASSERT(m_socket.get() != 0);
            m_socket->DisconnectFromHost(); 
        }

        void BindUser(ISbpTransportToUser *pUser)
        {
            TUT_ASSERT(pUser != 0);
            m_pUser = pUser;
        }

        std::string Info() const
        {
        	return "TestTcpTransport";
        }
        void Process(){}

        virtual size_t MaxSendSize() const { return 0; }


    public:
        // client-side constructor: create own socket
        // pUser can't be 0
        TestTcpTransport(iCore::MsgThread &thread, const Utils::HostInf &host,
                         ISbpTransportToUser *pUser)
        : m_socket(new TcpSocketEmul(thread, this)), m_pUser(pUser), m_pProto(0), m_host(host)
        {
            TUT_ASSERT(pUser != 0);
            TUT_ASSERT(m_socket.get() != 0);
            m_socket->LinkUserToSocket(this);
            Connect();
        }
        
        // server-side constructor: using given socket
        TestTcpTransport(boost::shared_ptr<ITcpSocketEmul> socket, 
                         ISbpTransportToUser *pUser = 0)
        : m_socket(socket), m_pUser(pUser), m_pProto(0)
        {
            TUT_ASSERT(socket.get() != 0);
            m_socket->LinkUserToSocket(this);
            m_host = socket->PeerHostInf();
        }
    };
    
    // ------------------------------------------------------------------
    
    // Test Tcp server
    class TestTcpTransportServer:
        public virtual Utils::SafeRefServer,
        public ITcpServerSocketEmulToUser,
        public ISbpUserTransportServer,
        public boost::noncopyable
    {
        ISbpUserTransportServerEvents &m_owner;
        TcpServerSocketEmul m_server;
        Utils::HostInf m_host;
        
    // ITcpServerEmulToUser
    private:
        void NewConnection(boost::shared_ptr<ITcpSocketEmul> socket)
        {
            TUT_ASSERT(socket.get() != 0);
            boost::shared_ptr<TestTcpTransport> transport(new TestTcpTransport(socket));
            m_owner.NewConnection(transport);
        }
        
        void ListenRej(boost::shared_ptr<SocketError> serverError)
        {
            TUT_ASSERT(serverError.get() != 0);
            m_owner.TransportServerError(serverError->ToString());
        }

    // ISbpUserTransportServer impl
    private:
        void Start() { m_server.Listen(m_host); }

    public:
        TestTcpTransportServer(ISbpUserTransportServerEvents &owner, 
                               iCore::MsgThread &thread,
                               const Utils::HostInf &host)
        : m_owner(owner), m_server(thread, this), m_host(host)
        {
        }
    };
    
} // namespace SBPTests

#endif
