#include "stdafx.h"
#include "Utils/ErrorsSubsystem.h"
#include "MsgTcpServer.h"
#include "TcpServer.h"
#include "MsgTcpSocket.h"
#include "SocketError.h"

namespace iNet 
{
    
    MsgTcpServer::MsgTcpServer(iCore::MsgThread &thread, Utils::SafeRef<ITcpServerToUser> user) : 
        iCore::MsgObject(thread), 
        m_user(user),
        m_tcpServer(new TcpServer(this)), 
        m_socketId(this)
        {}

	// ---------------------------------------------    

	MsgTcpServer::~MsgTcpServer()
	{
	}

    // ---------------------------------------------    

    bool MsgTcpServer::IsEqual(SocketId id)  const
    {
        return id == m_socketId; 
    }

    // ---------------------------------------------    

    void MsgTcpServer::NewConnection(SocketId id, boost::shared_ptr<ITcpSocket> socket)
    {
        PutMsg(this, &T::onNewConnection, socket);        
    }

    // ---------------------------------------------

    void MsgTcpServer::onNewConnection(boost::shared_ptr<ITcpSocket> socket)
    {
        boost::shared_ptr<MsgTcpSocket> msgSocket(new MsgTcpSocket(getMsgThread(), socket));
        m_user->NewConnection(m_socketId, msgSocket);
    }

    // ---------------------------------------------

    boost::shared_ptr<SocketError> MsgTcpServer::Listen(const Utils::HostInf &host)
    {
        return m_tcpServer->Listen(host);
    }

    // ---------------------------------------------
    
    void MsgTcpServer::Close()
    {
        m_tcpServer->Close();
    }

    // ---------------------------------------------
    
    Utils::HostInf MsgTcpServer::LocalHostInf() const
    {
        return m_tcpServer->LocalHostInf();
    }
   
}
