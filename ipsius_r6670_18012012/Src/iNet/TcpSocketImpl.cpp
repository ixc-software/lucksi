#include "stdafx.h"

#include "Utils/ErrorsSubsystem.h"
#include "Utils/QSyncDelete.h"
#include "NetThreadMonitor.h"
#include "ITcpSocketToUser.h"
#include "ITcpServerToUser.h"

#include "SocketData.h"
#include "HostInfAdapter.h"
#include "TcpSocketImpl.h"

namespace iNet
{
    // public methods

    TcpSocket::Impl::Impl(Utils::SafeRef<ITcpSocketToUser> iTcpSocketToUser) :
        m_memberThread(*this), m_isDeleted(false), m_isActive(false), m_isDisconnected(false)
    {
        InitBaseSignal();
        connect(this, SIGNAL(ConnectToHostSignal(Utils::HostInf)), 
            this, SLOT(ConnectToHostSlot(const Utils::HostInf&)));
        
        connect(this, SIGNAL(connected()), this, SLOT(ConnectedSlot()));

        LinkUserToSocket(iTcpSocketToUser);
    }

    // ----------------------------------------------------------------------------------------
	// server side

    TcpSocket::Impl::Impl() : 
        m_memberThread(*this), m_isDeleted(false), m_isActive(true), m_isDisconnected(false)
    {
        InitBaseSignal();
    }

    // ---------------------------------------------    

    bool TcpSocket::Impl::IsEqual(SocketId id) const  
    {
        return 1;//id == ID();
    }

    // ---------------------------------------------    

    void TcpSocket::Impl::ConnectToHost(const Utils::HostInf &host)
    {
        ESS_ASSERT(!m_user.IsEmpty());

        ConnectToHostSignal(host);
    }

    // ----------------------------------------------------------------------------------------

    void TcpSocket::Impl::SendDataSlot(const QByteArray &data)
    {
		NetThreadMonitor::TcpStat().IncWrite();
        if(!m_isActive || write(data) != -1) return;

        abort();       

        boost::shared_ptr<iNet::SocketError> 
            error(new iNet::SocketError(QAbstractSocket::UnknownSocketError, ""));
        m_user->Disconnected(error);
    }

    void TcpSocket::Impl::SendData( const QByteArray &data)
    {
        ESS_ASSERT(!m_user.IsEmpty());

        SendDataSignal(data);
    }

    // ----------------------------------------------------------------------------------------

    void TcpSocket::Impl::DisconnectFromHost()
    {
        ESS_ASSERT(!m_user.IsEmpty());
		
       
		DisconnectFromHostSignal();
    }

    // ----------------------------------------------------------------------------------------

    void TcpSocket::Impl::Delete()
    {
        // вызывается из потока пользователя
		ESS_ASSERT(!m_isDeleted);

        m_isDeleted.Set(true);        
        boost::shared_ptr<Utils::ThreadSyncEvent> lock( new Utils::ThreadSyncEvent() );
        DeleteSignal(lock); 
        lock->Wait();
    }

    // ----------------------------------------------------------------------------------------

    void TcpSocket::Impl::DeleteSlot(boost::shared_ptr<Utils::ThreadSyncEvent> lock)                    
    {        
		Utils::ThreadSyncEventUnlocker locker(*lock);
        if(!m_user.IsEmpty()) m_user.Clear();
        disconnect();
        abort();
        QTcpSocket::deleteLater();
    }

    // ----------------------------------------------------------------------------------------

    void TcpSocket::Impl::ConnectToHostSlot(const Utils::HostInf &host)
    {
        if(m_isDeleted) return;
		
		m_isDisconnected = false;
        QAbstractSocket:: SocketState state = AbstractSocket().state();        
        AbstractSocket().connectToHost(HostInfAdapter::Convert(host.Address()), host.Port());
		InitHostInf();
    }

    void TcpSocket::Impl::ConnectedSlot()
    {
        if(m_isDeleted) return;
		
		m_isActive.Set(true);
        InitHostInf();
        m_user->Connected(SocketId());
    }

    // ----------------------------------------------------------------------------------------

    void TcpSocket::Impl::ReceiveData()
    {
        if(m_isDeleted) return;
        
        if (m_user.IsEmpty()) return;

		NetThreadMonitor::TcpStat().IncRead();

        boost::shared_ptr<SocketData> data(new SocketData(PeerHostInf(), readAll()));

		if(data->getData().size() != 0) m_user->ReceiveData(data);
    }

    // ----------------------------------------------------------------------------------------

    void TcpSocket::Impl::DisconnectFromHostSlot()
    {
        abort();
    }

    // ----------------------------------------------------------------------------------------

    void TcpSocket::Impl::DisconnectedSlot()
    {
		boost::shared_ptr<iNet::SocketError>
			error(new iNet::SocketError(QAbstractSocket::RemoteHostClosedError, "Connection was closed"));
        Disconnected(error);
    }

    // ----------------------------------------------------------------------------------------

    void TcpSocket::Impl::SocketErrorOccur(QAbstractSocket::SocketError socketError)
    {
		boost::shared_ptr<iNet::SocketError>
			error(new iNet::SocketError(socketError, errorString()));

		Disconnected(error);
    }

    // ----------------------------------------------------------------------------------------

    bool TcpSocket::Impl::InitSocket(int socketDescriptor)
    {
        if(!QTcpSocket::setSocketDescriptor(socketDescriptor)) return 0;

        InitHostInf();
        return 1;
    }

    // ----------------------------------------------------------------------------------------

    void TcpSocket::Impl::LinkUserToSocket(Utils::SafeRef<ITcpSocketToUser> user)
    {
        ESS_ASSERT(!user.IsEmpty());
        ESS_ASSERT(m_user.IsEmpty());
        
        m_user = user;
        connect(this, SIGNAL(SendDataSignal(QByteArray)), 
            this, SLOT(SendDataSlot(const QByteArray&)));
        if(bytesAvailable ()) ReceiveData();
    }

    // ----------------------------------------------------------------------------------------

    void TcpSocket::Impl::InitHostInf()
    {
        m_localHostInf.Set(HostInfAdapter::Convert(localAddress()), localPort());
        m_peerHostInf.Set(HostInfAdapter::Convert(peerAddress()), peerPort());
    }

    // ----------------------------------------------------------------------------------------

    QString TcpSocket::Impl::getPeerName() const
    {
        return peerName();
    }

    // ----------------------------------------------------------------------------------------

    void TcpSocket::Impl::InitBaseSignal()
    {
        connect(this, SIGNAL(readyRead() ), this, SLOT(ReceiveData()));

        connect(this, SIGNAL(DisconnectFromHostSignal()), 
            this, SLOT(DisconnectFromHostSlot()));

        connect(this, SIGNAL(DeleteSignal(boost::shared_ptr<Utils::ThreadSyncEvent>)), 
            this, SLOT(DeleteSlot(boost::shared_ptr<Utils::ThreadSyncEvent>)));

        connect(this, SIGNAL(disconnected()), this, SLOT(DisconnectedSlot()));

        connect(this, SIGNAL(error(QAbstractSocket::SocketError)), 
            this, SLOT(SocketErrorOccur(QAbstractSocket::SocketError)));
    }

	// ----------------------------------------------------------------------------------------

	void TcpSocket::Impl::Disconnected(boost::shared_ptr<iNet::SocketError> error)
	{
		if(m_isDeleted) return;
		
		if(m_isDisconnected) return;
		
		m_isDisconnected = true;
		m_isActive.Set(false);

		if(!m_user.IsEmpty()) m_user->Disconnected(error);
	}
}











