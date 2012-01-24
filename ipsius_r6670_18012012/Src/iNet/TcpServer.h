#ifndef __TCP_SERVER_H__
#define __TCP_SERVER_H__

#include "stdafx.h"

#include "Utils/SafeRef.h"
#include "Utils/HostInf.h"

namespace iNet
{

    class ITcpServerToUser;
    class ITcpSocket;
	class SocketError;

    // Сервер TCP
    class TcpServer : boost::noncopyable
    {
    public:

        TcpServer(Utils::SafeRef<ITcpServerToUser> user);
        ~TcpServer();

        // Tells the server to listen for incoming connections on address address and port port.
        // If port is 0, a port is chosen automatically. If address is QHostAddress::Any,
        // the server will listen on all network interfaces.
        boost::shared_ptr<SocketError> Listen(const Utils::HostInf &host);
    
        // Closes the server. The server will no longer listen for incoming connections.
        void Close();
        
        Utils::HostInf LocalHostInf() const; 
	// Implementation TcpServer
	private:
		static boost::shared_ptr<ITcpSocket> CreateTcpSocket(int socketDescriptor);
		class Impl;
		Impl *m_impl;

    };
}

#endif

