#ifndef __TELNETSERVER__
#define __TELNETSERVER__

#include "ITelnet.h"
#include "TelnetSockets.h"
#include "iNet/ITcpServerToUser.h"

#include "iCore/MsgObject.h"
#include "iCore/MsgThread.h"

namespace Telnet
{
    using iCore::MsgThread;
    using iCore::MsgObject;
    using Utils::SafeRef;
    using boost::shared_ptr;


    // server side class 
    // used to establish connection with client
    class TelnetServer : public MsgObject, 
                         public iNet::ITcpServerToUser,
                         public boost::noncopyable
    {
        SafeRef<ITelnetServerEvents> m_pOwner;
        shared_ptr<TelnetServerSocket> m_pSocket;
    
    // implement ITcpServerToUser
    private:
        void NewConnection(iNet::SocketId id, boost::shared_ptr<iNet::ITcpSocket> socket);
    public:
        TelnetServer(MsgThread &thread, 
                     SafeRef<ITelnetServerEvents> owner,
                     const Utils::HostInf &host);
		     
        Utils::HostInf LocalHostInf() const;
    };

} // namespace Telnet

#endif
