
#include "stdafx.h"

#include "TcpServerSocketEmul.h"


namespace TelnetTests
{

    TcpServerSocketEmul::TcpServerSocketEmul(iCore::MsgThread &thread, 
                        Utils::SafeRef<iNet::ITcpServerToUser> user)
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
        pServer->setOther(pClient);
        pClient->setOther(pServer.get());
        pServer->ConnectToHost(pClient->LocalHostInf()); 
        pClient->Connected();
        // pServer->Connected();

        m_pOwner->NewConnection(iNet::SocketId(this), pServer);
    }

    // -------------------------------------------------

    void TcpServerSocketEmul::OnListen(boost::shared_ptr<Utils::HostInf> hostInf)
    {
        m_localInf = *hostInf;
        LocalSocketRegister().Register(this);
    }

    // -------------------------------------------------
    // implement ITcpServerSocket

    const Utils::HostInf& TcpServerSocketEmul::LocalInf() const { return m_localInf; }

    // -------------------------------------------------

    void TcpServerSocketEmul::NewConnection(Utils::SafeRef<ITcpSocketEmul> pClient)
    {
        PutMsg(this, &TcpServerSocketEmul::OnNewConnection, pClient);
    }

} // namespace TelnetTests
