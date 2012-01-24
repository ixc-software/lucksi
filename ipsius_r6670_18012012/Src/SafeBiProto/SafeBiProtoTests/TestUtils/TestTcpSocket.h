#ifndef __TESTTCPSOCKET__
#define __TESTTCPSOCKET__

#include "iCore/MsgObject.h"
#include "iCore/MsgThread.h"
#include "Utils/HostInf.h"

#include "LocalTestSocketRegister.h"


namespace TestUtils
{
    // test-socket class
    class TcpSocketEmul : public iCore::MsgObject,
                          public ITcpSocketEmul,
                          public boost::noncopyable
    {
        typedef TcpSocketEmul T;

        Utils::SafeRef<ITcpSocketEmulToUser> m_pOwner;
        Utils::SafeRef<ITcpSocketEmul> m_pOtherSide;
        Utils::HostInf m_peerInf;
        Utils::HostInf m_localInf;
        bool m_connected;

        // events
        void OnConnectToHost(boost::shared_ptr<Utils::HostInf> pIntf);
        void OnDisconnected();
        void OnSendData(boost::shared_ptr<Utils::ManagedMemBlock> pData);
        // void ReceiveData(const QByteArray &data);
        
    // implement ITcpSocketEmul
    private:
        void Connected();
        void OnReceiveData(boost::shared_ptr<Utils::ManagedMemBlock> pData);
        void resetOther();
        bool IsConnected();
        const Utils::HostInf& PeerHostInf() const { return m_peerInf; }  
        const Utils::HostInf &LocalHostInf() const { return m_localInf; }

    // implement ITcpSocketEmul
    public:
        void LinkUserToSocket(Utils::SafeRef<ITcpSocketEmulToUser> iTcpSocketToUser);
        void ConnectToHost(const Utils::HostInf &host);
        void SendData(const void *pData, size_t size);
        void DisconnectFromHost();
        void setOther(Utils::SafeRef<ITcpSocketEmul> pOther);

    public:
        TcpSocketEmul(iCore::MsgThread &thread, 
                      Utils::SafeRef<ITcpSocketEmulToUser> iTcpSocketToUser);
        TcpSocketEmul(iCore::MsgThread &thread);
        ~TcpSocketEmul();
    };

} // namespace TestUtils

#endif
