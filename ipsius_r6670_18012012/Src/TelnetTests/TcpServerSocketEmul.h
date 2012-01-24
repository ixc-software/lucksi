#ifndef __TCPSERVERSOCKETEMUL__
#define __TCPSERVERSOCKETEMUL__

#include "iCore/MsgObject.h"
#include "iCore/MsgThread.h"
#include "iNet/ITcpSocket.h"
#include "iNet/ITcpServerToUser.h"
#include "iNet/ISocketId.h"
#include "Utils/HostInf.h"

#include "TcpSocketEmul.h"
#include "LocalSocketRegister.h"

namespace TelnetTests
{
    // test-server socket class
    class TcpServerSocketEmul : public iCore::MsgObject,
                                public iNet::ISocketId,
                                public ITcpServerSocketEmul,
                                public boost::noncopyable
    {
        iCore::MsgThread &m_thread;
        Utils::SafeRef<iNet::ITcpServerToUser> m_pOwner;
        Utils::HostInf m_localInf;

        // events
        void OnNewConnection(Utils::SafeRef<ITcpSocketEmul> pClient);
        void OnListen(boost::shared_ptr<Utils::HostInf> hostInf);

    public:
        TcpServerSocketEmul(iCore::MsgThread &thread, 
                            Utils::SafeRef<iNet::ITcpServerToUser> user);
        ~TcpServerSocketEmul();

        void Listen(const Utils::HostInf &host);
        void Close();

    // implement ITcpServerSocket
    public:
        const Utils::HostInf &LocalInf() const;
        void NewConnection(Utils::SafeRef<ITcpSocketEmul> pClient);
    };
 
} // TelnetTest

#endif
