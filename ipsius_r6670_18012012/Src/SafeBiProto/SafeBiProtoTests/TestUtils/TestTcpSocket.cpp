
#include "stdafx.h"

#include "TestTcpSocket.h"


namespace TestUtils
{
    TcpSocketEmul::TcpSocketEmul(iCore::MsgThread &thread, 
                                 Utils::SafeRef<ITcpSocketEmulToUser> iTcpSocketToUser)
    : iCore::MsgObject(thread), m_pOwner(iTcpSocketToUser), m_connected(false)
    {}

    // ---------------------------------------------------

    TcpSocketEmul::TcpSocketEmul(iCore::MsgThread &thread)
    : iCore::MsgObject(thread), m_connected(false)
    {}

    // ---------------------------------------------------

    TcpSocketEmul::~TcpSocketEmul()
    {
        if (m_pOtherSide.IsEmpty()) return;

        m_pOtherSide->resetOther();
    }

    // ---------------------------------------------------
    // events
    void TcpSocketEmul::OnConnectToHost(boost::shared_ptr<Utils::HostInf> pIntf)
    {
        m_peerInf = *pIntf;
        LocalSocketRegister().Link(this, *pIntf);
    }

    // ---------------------------------------------------

    void TcpSocketEmul::OnDisconnected()
    {
        m_connected = false;

        if (!m_pOtherSide.IsEmpty())
        {
            if (m_pOtherSide->IsConnected()) m_pOtherSide->DisconnectFromHost();
        }
            
        boost::shared_ptr<SocketError> error; 

        if (!m_pOwner.IsEmpty()) m_pOwner->Disconnected(error);
    }

    // ---------------------------------------------------

    void TcpSocketEmul::OnSendData(boost::shared_ptr<Utils::ManagedMemBlock> pData)
    {
        ESS_ASSERT(!m_pOtherSide.IsEmpty());
        m_pOtherSide->OnReceiveData(pData);
    }

    // ---------------------------------------------------
    /*
    void TcpSocketEmul::ReceiveData(const QByteArray &data)
    {
        PutMsg(this, &T::OnReceiveData, shared_ptr<QByteArray>(new QByteArray(data)));
    }
    */

    // ---------------------------------------------------
    // implement ITcpSocketEmul
    void TcpSocketEmul::Connected()
    {
        // m_connected = true;
        m_pOwner->Connected();
    }

    // ---------------------------------------------------

    void TcpSocketEmul::OnReceiveData(boost::shared_ptr<Utils::ManagedMemBlock> pData)
    {
        m_pOwner->ReceiveData(pData);
    }

    // ---------------------------------------------------

    void TcpSocketEmul::resetOther()
    {
        m_pOtherSide.Clear();
    }

    // ---------------------------------------------------

    bool TcpSocketEmul::IsConnected() { return m_connected; }

    // ---------------------------------------------------
    // implement ITcpSocket
    void TcpSocketEmul::LinkUserToSocket(Utils::SafeRef<ITcpSocketEmulToUser> iTcpSocketToUser) 
    {
        ESS_ASSERT(!iTcpSocketToUser.IsEmpty());
        m_pOwner = iTcpSocketToUser;
    }

    // ---------------------------------------------------

    void TcpSocketEmul::ConnectToHost(const Utils::HostInf &host)
    {
        m_connected = true;
        m_peerInf = host;
        PutMsg(this, 
               &T::OnConnectToHost, 
               boost::shared_ptr<Utils::HostInf>(new Utils::HostInf(host)));
    }

    // ---------------------------------------------------

    void TcpSocketEmul::SendData(const void *pData, size_t size)
    {
        if (!m_connected) return;
        
        PutMsg(this, &T::OnSendData, boost::shared_ptr<Utils::ManagedMemBlock>(
                                        new Utils::ManagedMemBlock(pData, size)));
    }

    // ---------------------------------------------------

    void TcpSocketEmul::DisconnectFromHost()
    {
        PutMsg(this, &T::OnDisconnected);
    }

    // ---------------------------------------------------
    // implement ITcpSocketEmul
    void TcpSocketEmul::setOther(Utils::SafeRef<ITcpSocketEmul> pOther) 
    { 
        m_pOtherSide = pOther;
    }

} // namespace TestUtils
