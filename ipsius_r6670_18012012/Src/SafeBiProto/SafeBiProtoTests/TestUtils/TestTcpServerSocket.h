#ifndef __TESTTCPSERVERSOCKET__
#define __TESTTCPSERVERSOCKET__

#include "iCore/MsgObject.h"
#include "iCore/MsgThread.h"

#include "TestTcpSocket.h"
#include "LocalTestSocketRegister.h"

namespace TestUtils
{
    class ITcpServerSocketEmulToUser : public Utils::IBasicInterface
    {
    public:
        virtual void NewConnection(boost::shared_ptr<ITcpSocketEmul> socket) = 0;
        virtual void ListenRej(boost::shared_ptr<SocketError> serverError) = 0;
    };

    // -----------------------------------------------------------
    
    // test-server socket class
    class TcpServerSocketEmul : public iCore::MsgObject,
                                public ITcpServerSocketEmul,
                                public boost::noncopyable
    {
        iCore::MsgThread &m_thread;
        Utils::SafeRef<ITcpServerSocketEmulToUser> m_pOwner;
        Utils::HostInf m_localInf;

        // events
        void OnNewConnection(Utils::SafeRef<ITcpSocketEmul> pClient);
        void OnListen(boost::shared_ptr<Utils::HostInf> hostInf);
        void OnListenRej(boost::shared_ptr<SocketError> serverError);

    public:
        TcpServerSocketEmul(iCore::MsgThread &thread, 
                            Utils::SafeRef<ITcpServerSocketEmulToUser> user);
        ~TcpServerSocketEmul();

        void Listen(const Utils::HostInf &host);
        void Close();

    public:
        const Utils::HostInf &LocalInf() const { return m_localInf; }
        void NewConnection(Utils::SafeRef<ITcpSocketEmul> pClient);
    };
 
     
} // TestUtils

#endif
