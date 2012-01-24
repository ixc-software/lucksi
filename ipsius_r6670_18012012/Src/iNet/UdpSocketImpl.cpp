#include "stdafx.h"
#include "Utils/QSyncDelete.h"
#include "NetThreadMonitor.h"
#include "SocketData.h"
#include "SocketError.h"
#include "IUdpSocketToUser.h"
#include "UdpException.h"
#include "UdpSocketImpl.h"
#include "HostInfAdapter.h"

namespace iNet
{
    class BindEventUdpSocket : boost::noncopyable
    {
    public:
        BindEventUdpSocket(const Utils::HostInf &host) : 
            m_host(host) 
        {}

        void Wait()
        {
            m_lock.Wait();
        }

        void Ready()
        {
            m_lock.Ready();
        }
        
        const Utils::HostInf &getHostInf() const 
        {
            return m_host;
        }

    private:
        Utils::HostInf m_host;
        Utils::ThreadSyncEvent m_lock;
    };

    // ----------------------------------------------------------------------------------------        

    UdpSocket::Impl::Impl(Utils::SafeRef<IUdpSocketToUser> iUdpSocketToUser, const Utils::HostInf &host) :
        m_memberThread(*this), 
        m_user(iUdpSocketToUser),
        m_isBinded(false)
    {
        ESS_ASSERT(!host.Empty());

        ConnectSignalToSlot();
        if(!Bind(host)) ESS_THROW(UdpBindExcept);
    }
    // ----------------------------------------------------------------------------------------    

    UdpSocket::Impl::Impl(Utils::SafeRef<IUdpSocketToUser> iUdpSocketToUser) :
        m_memberThread(*this), 
        m_user(iUdpSocketToUser),
        m_isBinded(false)
    {
        ConnectSignalToSlot();
    }

    // ----------------------------------------------------------------------------------------        

    bool UdpSocket::Impl::Bind(const Utils::HostInf &host) 
    {
        ESS_ASSERT(!host.Empty());
		boost::shared_ptr<Utils::ThreadSyncEvent> lock( new Utils::ThreadSyncEvent() );
		BindSignal(lock, host);
		lock->Wait();
        return m_isBinded;
    }

    // ----------------------------------------------------------------------------------------        

    bool UdpSocket::Impl::BindImpl(const Utils::HostInf &host) 
    {
		m_localHostInf = host;
		disconnect(this, SIGNAL(error(QAbstractSocket::SocketError)), 
			this, SLOT(ErrorOccur(QAbstractSocket::SocketError)));

        m_isBinded = bind(HostInfAdapter::Convert(host.Address()), host.Port());

		connect(this, SIGNAL(error(QAbstractSocket::SocketError)), 
			this, SLOT(ErrorOccur(QAbstractSocket::SocketError)));

        return m_isBinded;
    }

    // ----------------------------------------------------------------------------------------        

    void UdpSocket::Impl::BindSlot(boost::shared_ptr<Utils::ThreadSyncEvent> lock, const Utils::HostInf &host)
    {
		Utils::ThreadSyncEventUnlocker locker(*lock);
        BindImpl(host);
    }

    // ----------------------------------------------------------------------------------------    

    void UdpSocket::Impl::ConnectSignalToSlot()
    {
		connect(this, SIGNAL(BindSignal(boost::shared_ptr<Utils::ThreadSyncEvent>, Utils::HostInf)), 
			this, SLOT(BindSlot(boost::shared_ptr<Utils::ThreadSyncEvent>, const Utils::HostInf &)));

        connect(this, SIGNAL(SendDataSignal(boost::shared_ptr<iNet::SocketData>)), 
            this, SLOT(SendDataSlot(boost::shared_ptr<iNet::SocketData>)));

        connect(this, SIGNAL(readyRead() ), this, SLOT(ReceiveDataSlot()));

        connect(this, SIGNAL(error(QAbstractSocket::SocketError)), 
            this, SLOT(ErrorOccur(QAbstractSocket::SocketError)));

        connect(this, SIGNAL(DeleteSignal(boost::shared_ptr<Utils::ThreadSyncEvent>)), 
            this, SLOT(DeleteSlot(boost::shared_ptr<Utils::ThreadSyncEvent>)));        
    }
    
    // ----------------------------------------------------------------------------------------

    void UdpSocket::Impl::SendData(boost::shared_ptr<SocketData> data)
    {
        ESS_ASSERT(m_isBinded);

        SendDataSignal(data);
    }

    // ----------------------------------------------------------------------------------------

    void UdpSocket::Impl::SendDataSlot(boost::shared_ptr<SocketData> data)
    {
		NetThreadMonitor::UdpStat().IncWrite();
        qint64 len = writeDatagram (data->getData(), 
            HostInfAdapter::Convert((data->getHostinf()).Address()),
            (data->getHostinf()).Port());
    }

    // ----------------------------------------------------------------------------------------

    void UdpSocket::Impl::ReceiveDataSlot()
    {
        if(isDeleted()) return;

        if (!hasPendingDatagrams()) return;
		NetThreadMonitor::UdpStat().IncRead();
        QByteArray datagram;
        datagram.resize(pendingDatagramSize());


        QHostAddress sender;
        quint16 senderPort;
       
        readDatagram(datagram.data(), datagram.size(), &sender, &senderPort);
        Utils::HostInf host(HostInfAdapter::Convert(sender), senderPort);

        boost::shared_ptr<SocketData> data(new SocketData(host, datagram));
        m_user->ReceiveData(data);
    }
    
    // ----------------------------------------------------------------------------------------

    void UdpSocket::Impl::ErrorOccur(QAbstractSocket::SocketError socketError)
    {
        if (isDeleted()) return;

        abort();

        boost::shared_ptr<iNet::SocketError> error(new iNet::SocketError(socketError, errorString()));

        m_user->SocketErrorOccur(error);
    }
    
    // ----------------------------------------------------------------------------------------
    
    void UdpSocket::Impl::Delete()
    {
        // вызывается из потока пользователя
        m_isDeleted.Set(true);
        boost::shared_ptr<Utils::ThreadSyncEvent> lock( new Utils::ThreadSyncEvent() );
        DeleteSignal(lock); 
        lock->Wait();
    }
    
    // ----------------------------------------------------------------------------------------

    void UdpSocket::Impl::DeleteSlot(boost::shared_ptr<Utils::ThreadSyncEvent> lock)                    
    {        
        m_user.Clear();
        disconnect();
        abort();
        QUdpSocket::deleteLater();
        lock->Ready();
    }
    
    // ----------------------------------------------------------------------------------------
    
    const Utils::HostInf &UdpSocket::Impl::LocalHostInf() const
    {
        ESS_ASSERT(m_isBinded);

        return m_localHostInf;
    }
}
