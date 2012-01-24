#ifndef __TCPSOCKETEMUL__
#define __TCPSOCKETEMUL__

#include "iCore/MsgObject.h"
#include "iCore/MsgThread.h"
#include "Utils/Random.h"

#include "iNet/ITcpSocketToUser.h"
#include "iNet/ISocketId.h"
#include "iNet/ITcpSocket.h"
#include "iNet/SocketData.h"
#include "Utils/HostInf.h"

#include "LocalSocketRegister.h"


namespace TelnetTests
{
    //using namespace iNet;
    using boost::shared_ptr;

    // test-socket class
    class TcpSocketEmul : public iCore::MsgObject,
                          public iNet::ISocketId,
                          public ITcpSocketEmul,
                          public boost::noncopyable
    {
        typedef TcpSocketEmul T;

        Utils::SafeRef<iNet::ITcpSocketToUser> m_pOwner;
        Utils::HostInf m_localInf;
        Utils::HostInf m_peerInf;
        QString m_peerName;
        iNet::SocketId m_socketId;
        Utils::SafeRef<ITcpSocketEmul> m_pOtherSide;
        bool m_connected;

        // events
        void OnConnectToHost(boost::shared_ptr<Utils::HostInf> pIntf);
        void OnDisconnected();
        void OnSendData(boost::shared_ptr<QByteArray> pData);
        // void ReceiveData(const QByteArray &data);
        
    // implement ITcpSocketEmul
    private:
        void Connected();
        void OnReceiveData(boost::shared_ptr<QByteArray> pData);
        void resetOther();
        bool IsConnected();

    public:
        TcpSocketEmul(iCore::MsgThread &thread, 
            Utils::SafeRef<iNet::ITcpSocketToUser> iTcpSocketToUser);
        TcpSocketEmul(iCore::MsgThread &thread);
        ~TcpSocketEmul();

    // implement ITcpSocket
    public:
        void LinkUserToSocket(Utils::SafeRef<iNet::ITcpSocketToUser> iTcpSocketToUser);
        void ConnectToHost(const Utils::HostInf &host);
        void SendData(const QByteArray &data);
        void DisconnectFromHost();

        const Utils::HostInf &LocalHostInf() const;
        const Utils::HostInf &PeerHostInf() const;
        QString getPeerName () const;
        bool IsEqual(iNet::SocketId id) const;
        
    // implement ITcpSocketEmul
    public:
        void setOther(Utils::SafeRef<ITcpSocketEmul> pOther);
    };

} // namespace TelnetTest

#endif
