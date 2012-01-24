#ifndef __TCP_SOCKET_IMPL_H__
#define __TCP_SOCKET_IMPL_H__

#include "stdafx.h"
#include "MemberSocketThread.h"
#include "Utils/HostInf.h"
#include "ITcpSocket.h"
#include "SocketError.h"
#include "TcpSocket.h"


namespace Utils 
{
    class ThreadSyncEvent;
};

namespace iNet 
{
    
    class ITcpServerToUser;

    class TcpSocket::Impl : 
        private QTcpSocket,
        public ITcpSocket
    {
        Q_OBJECT    

    public:

        Impl(Utils::SafeRef<ITcpSocketToUser> iTcpSocketToUser);
        Impl();
        
        bool InitSocket(int socketDescriptor);
        void Delete();
    // реализация ITcpSocket
    public:            
        bool IsEqual(SocketId id) const;
        void LinkUserToSocket(Utils::SafeRef<ITcpSocketToUser> iTcpSocketToUser);
        void ConnectToHost(const Utils::HostInf &host);
        void SendData(const QByteArray &data);

        void DisconnectFromHost();

        const Utils::HostInf &LocalHostInf() const 
        {
            return m_localHostInf;
        }

        const Utils::HostInf &PeerHostInf() const 
        {
            return m_peerHostInf;
        }
        
        QString getPeerName () const;
        // end of реализация ITcpSocket

    private slots:
        void ConnectToHostSlot (const Utils::HostInf &host);
        void ConnectedSlot();

        void SendDataSlot(const QByteArray &data);                
        void ReceiveData();
        void DisconnectFromHostSlot();
        void DisconnectedSlot();
        void SocketErrorOccur(QAbstractSocket::SocketError socketError);    
        void DeleteSlot(boost::shared_ptr<Utils::ThreadSyncEvent>);            
    signals:
        void SendDataSignal(QByteArray data);        
        void ConnectToHostSignal(Utils::HostInf host);
        void DisconnectFromHostSignal();
        void DeleteSignal(boost::shared_ptr<Utils::ThreadSyncEvent>);            
    private:
		void Disconnected(boost::shared_ptr<iNet::SocketError> error);
        QAbstractSocket &AbstractSocket() 
        {
            return *this;
        }
    private:        
        MemberSocketThread m_memberThread;        
        
        Utils::SafeRef<ITcpSocketToUser> m_user;
        Utils::AtomicBool m_isDeleted;
        Utils::AtomicBool m_isActive;
        bool m_isDisconnected;
        void InitHostInf();
        void InitBaseSignal();

        Utils::HostInf m_localHostInf;
        Utils::HostInf m_peerHostInf;
    };  // end of TcpSocketImpl

};

#endif

