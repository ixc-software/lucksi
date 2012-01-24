
#include "stdafx.h"

#include "TestTcpServerSocket.h"


namespace TestUtils
{

    TcpServerSocketEmul::TcpServerSocketEmul(iCore::MsgThread &thread, 
                                             Utils::SafeRef<ITcpServerSocketEmulToUser> user)
    : iCore::MsgObject(thread), m_thread(thread), m_pOwner(user)
    {}

    // -------------------------------------------------

    TcpServerSocketEmul::~TcpServerSocketEmul()
    {
        LocalSocketRegister().UnRegister(this);
    }

    // -------------------------------------------------

    void TcpServerSocketEmul::Listen(const Utils::HostInf &host)
    {
        PutMsg(this, &TcpServerSocketEmul::OnListen,
               boost::shared_ptr<Utils::HostInf>(new Utils::HostInf(host)));
    }

    // -------------------------------------------------

    void TcpServerSocketEmul::Close() {}

    // -------------------------------------------------
    // events
    void TcpServerSocketEmul::OnNewConnection(Utils::SafeRef<ITcpSocketEmul> pClient)
    {
        boost::shared_ptr<TcpSocketEmul> pServer(new TcpSocketEmul(m_thread));
        TUT_ASSERT(pServer.get() != 0);
        pServer->setOther(pClient);
        pClient->setOther(pServer.get());
        pServer->ConnectToHost(pClient->LocalHostInf()); 
        pClient->Connected();
        // pServer->Connected();

        m_pOwner->NewConnection(pServer);
    }

    // -------------------------------------------------

    void TcpServerSocketEmul::OnListen(boost::shared_ptr<Utils::HostInf> hostInf)
    {
        m_localInf = *hostInf;
        LocalSocketRegister().Register(this);
    }

    // -------------------------------------------------

    void TcpServerSocketEmul::OnListenRej(boost::shared_ptr<SocketError> serverError)
    {
        m_pOwner->ListenRej(serverError);
    }

    // -------------------------------------------------
    
    void TcpServerSocketEmul::NewConnection(Utils::SafeRef<ITcpSocketEmul> pClient)
    {
        PutMsg(this, &TcpServerSocketEmul::OnNewConnection, pClient);
    }

   
} // namespace TestUtils
