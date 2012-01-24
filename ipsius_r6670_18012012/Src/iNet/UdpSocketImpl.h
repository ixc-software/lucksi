#ifndef __UDP_SOCKET_IMPL_H__
#define __UDP_SOCKET_IMPL_H__

#include "stdafx.h"
#include "MemberSocketThread.h"
#include "Utils/HostInf.h"
#include "Utils/ThreadSyncEvent.h"
#include "UdpSocket.h"

namespace Utils 
{
    class ThreadSyncEvent;
};

namespace iNet
{
    class IUdpSocketToUser;
    using Utils::SafeRef;
//    class BindEventUdpSocket;        

    class UdpSocket::Impl : private QUdpSocket
    {
        Q_OBJECT
    public:
        Impl(SafeRef<IUdpSocketToUser> iUdpSocketToUser, 
            const Utils::HostInf &host); // !!! can throw UdpBindExcept
        
        Impl(SafeRef<IUdpSocketToUser> iUdpSocketToUser);

        bool Bind(const Utils::HostInf &host);

        void SendData(boost::shared_ptr<SocketData> data);
        void Delete();
        const Utils::HostInf &LocalHostInf() const;          

    private slots:
        void BindSlot(boost::shared_ptr<Utils::ThreadSyncEvent> lock, const Utils::HostInf &host);
        void SendDataSlot(boost::shared_ptr<iNet::SocketData> data);
        void ReceiveDataSlot();
        void ErrorOccur(QAbstractSocket::SocketError socketError);
        void DeleteSlot(boost::shared_ptr<Utils::ThreadSyncEvent>);
    signals:
        void BindSignal(boost::shared_ptr<Utils::ThreadSyncEvent> lock, const Utils::HostInf host);
        void SendDataSignal(boost::shared_ptr<iNet::SocketData> data);
        void DeleteSignal(boost::shared_ptr<Utils::ThreadSyncEvent>);            
    private:
        bool isDeleted() const 
        {
            return m_isDeleted.Get();
        }
        void ConnectSignalToSlot();

        bool BindImpl(const Utils::HostInf &host);
    private:
        Utils::HostInf m_localHostInf;
        MemberSocketThread m_memberThread;
        Utils::AtomicBool m_isDeleted;
        Utils::SafeRef<IUdpSocketToUser> m_user;
        bool m_isBinded;


    };  // end of TcpSocketImpl
};

#endif

