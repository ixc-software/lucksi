
#include "stdafx.h"

#include "TcpSocketEmul.h"


namespace TelnetTests
{
    TcpSocketEmul::TcpSocketEmul(iCore::MsgThread &thread, 
                  Utils::SafeRef<iNet::ITcpSocketToUser> iTcpSocketToUser)
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
            
        boost::shared_ptr<iNet::SocketError> error; 

        if (!m_pOwner.IsEmpty()) 
            m_pOwner->Disconnected(error);
    }

    // ---------------------------------------------------

    void TcpSocketEmul::OnSendData(boost::shared_ptr<QByteArray> pData)
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
        m_pOwner->Connected(iNet::SocketId(this));
    }

    // ---------------------------------------------------

    void TcpSocketEmul::OnReceiveData(boost::shared_ptr<QByteArray> pData)
    {
        m_pOwner->ReceiveData(shared_ptr<iNet::SocketData>(new iNet::SocketData(/*SocketId(this),*/
                                                                    m_localInf, 
                                                                    *pData)));
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
    void TcpSocketEmul::LinkUserToSocket(Utils::SafeRef<iNet::ITcpSocketToUser> iTcpSocketToUser) 
    {
        ESS_ASSERT(!iTcpSocketToUser.IsEmpty());
        m_pOwner = iTcpSocketToUser;
    }

    // ---------------------------------------------------

    void TcpSocketEmul::ConnectToHost(const Utils::HostInf &host)
    {
        m_connected = true;
        PutMsg(this, 
               &T::OnConnectToHost, 
               shared_ptr<Utils::HostInf>(new Utils::HostInf(host)));
    }

    // ---------------------------------------------------

    void TcpSocketEmul::SendData(const QByteArray &data)
    {
        if (!m_connected) return;
        PutMsg(this, &T::OnSendData, shared_ptr<QByteArray>(new QByteArray(data)));
    }

    // ---------------------------------------------------

    void TcpSocketEmul::DisconnectFromHost()
    {
        PutMsg(this, &T::OnDisconnected);
    }

    const Utils::HostInf& TcpSocketEmul::LocalHostInf() const { return m_localInf; }

    // ---------------------------------------------------

    const Utils::HostInf& TcpSocketEmul::PeerHostInf() const { return m_peerInf; }  

    // ---------------------------------------------------

    QString TcpSocketEmul::getPeerName () const { return m_peerName; }

    // ---------------------------------------------------

    bool TcpSocketEmul::IsEqual(iNet::SocketId id) const { return id == m_socketId; }

    // ---------------------------------------------------
    // implement ITcpSocketEmul
    void TcpSocketEmul::setOther(Utils::SafeRef<ITcpSocketEmul> pOther) 
    { 
        m_pOtherSide = pOther;
    }

} // namespace TelnetTests
