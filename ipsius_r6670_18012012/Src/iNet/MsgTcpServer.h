#ifndef __MSG_TCP_SERVER_H__
#define __MSG_TCP_SERVER_H__

#include "stdafx.h"
#include "iCore/MsgThread.h"
#include "iCore/MsgObject.h"

#include "Utils/HostInf.h"
#include "ISocketId.h"
#include "ITcpServerToUser.h"

namespace iNet 
{
    class SocketError;
    class TcpServer;
    class ITcpSocket;

    // TcpServer, работающий через очередь особщений
    class MsgTcpServer : 
        boost::noncopyable,
        public iCore::MsgObject, 
        public ISocketId,
        public ITcpServerToUser
    {
        typedef MsgTcpServer T;    
    public:
        MsgTcpServer(iCore::MsgThread &thread, Utils::SafeRef<ITcpServerToUser> user);
		~MsgTcpServer();

        bool IsEqual(SocketId id)  const;

        // Tells the server to listen for incoming connections on address address and port port. 
        // If port is 0, a port is chosen automatically. If address is QHostAddress::Any, 
        // the server will listen on all network interfaces.
        boost::shared_ptr<SocketError> Listen(const Utils::HostInf &host);

        // Closes the server. The server will no longer listen for incoming connections.
        void Close();
        
        Utils::HostInf LocalHostInf() const; 
	// ITcpServerToUser
	private:    
		void NewConnection(SocketId id, boost::shared_ptr<ITcpSocket> socket);
	// ITcpServerToUser 
	private:    
		void onNewConnection(boost::shared_ptr<ITcpSocket> socket);
	private:    
        Utils::SafeRef<ITcpServerToUser> m_user;      
		boost::scoped_ptr<TcpServer> m_tcpServer;
		SocketId m_socketId;
    };
}

#endif

