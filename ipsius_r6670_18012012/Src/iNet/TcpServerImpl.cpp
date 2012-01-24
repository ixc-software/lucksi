
#include "stdafx.h"
#include "Utils/QSyncDelete.h"

#include "ITcpServerToUser.h"
#include "ITcpSocketToUser.h"
#include "SocketError.h"
#include "HostInfAdapter.h"
#include "ITcpSocket.h"
#include "TcpServerImpl.h"

namespace iNet
{

    TcpServer::Impl::Impl(Utils::SafeRef<ITcpServerToUser> user) :
        m_memberThread(*this),
        m_user(user)
    {
        connect(this, SIGNAL(ListenSignal(boost::shared_ptr<Utils::ThreadSyncEvent>, Utils::HostInf)), 
            this, SLOT(ListenSlot(boost::shared_ptr<Utils::ThreadSyncEvent>, const Utils::HostInf &)));

        connect(this, SIGNAL(DeleteSignal(boost::shared_ptr<Utils::ThreadSyncEvent>)), 
            this, SLOT(DeleteSlot(boost::shared_ptr<Utils::ThreadSyncEvent>)));
    }
    
    // ----------------------------------------------------------------------------------------
    
    TcpServer::Impl::~Impl()
    {}
    
    // ----------------------------------------------------------------------------------------

    boost::shared_ptr<SocketError> TcpServer::Impl::Listen(const Utils::HostInf &host)
    {
		ESS_ASSERT(m_error == 0);

		boost::shared_ptr<Utils::ThreadSyncEvent> lock( new Utils::ThreadSyncEvent() );

        ListenSignal(lock, host);

		lock->Wait();
		boost::shared_ptr<SocketError> error;
		error.swap(m_error);
		return error;
    }

	// ----------------------------------------------------------------------------------------

	void TcpServer::Impl::ListenSlot(boost::shared_ptr<Utils::ThreadSyncEvent> lock, const Utils::HostInf &host)
	{
		ESS_ASSERT(m_error == 0);
		
		Utils::ThreadSyncEventUnlocker locker(*lock);

		while (!isListening() && !listen(HostInfAdapter::Convert(host.Address()), host.Port()))
		{
			m_error.reset(new SocketError(serverError(), errorString()));
			return;
		}
		m_hostInf.Set(HostInfAdapter::Convert(serverAddress()), serverPort());
	}

    // ----------------------------------------------------------------------------------------

    void TcpServer::Impl::Delete()
    {
        boost::shared_ptr<Utils::ThreadSyncEvent> lock( new Utils::ThreadSyncEvent() );
        DeleteSignal(lock); 
        lock->Wait();
    }

    // ----------------------------------------------------------------------------------------

    void TcpServer::Impl::DeleteSlot(boost::shared_ptr<Utils::ThreadSyncEvent> lock)                    
    {        
        m_user.Clear();
        disconnect();
        close();
        QTcpServer::deleteLater();
        lock->Ready();
    }

    // ----------------------------------------------------------------------------------------

    void TcpServer::Impl::Close()
    {
        close();
    }

    // ----------------------------------------------------------------------------------------

    void TcpServer::Impl::incomingConnection (int socketDescriptor)
    {
        boost::shared_ptr<ITcpSocket> tcpSocket = TcpServer::CreateTcpSocket(socketDescriptor);
        SocketId id;
        if(tcpSocket.get()) m_user->NewConnection(id, tcpSocket);
    }

}
