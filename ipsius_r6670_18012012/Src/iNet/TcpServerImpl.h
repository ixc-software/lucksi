#ifndef __TCP_SERVER_IMPL_H__
#define __TCP_SERVER_IMPL_H__

#include "stdafx.h"

#include "MemberSocketThread.h"
#include "Utils/HostInf.h"
#include "TcpServer.h"

namespace Utils 
{
    class ThreadSyncEvent;
};

namespace iNet
{
    class ITcpServerToUser;

    class TcpServer::Impl : private QTcpServer
    {
        Q_OBJECT

    public:

        Impl(Utils::SafeRef<ITcpServerToUser> user);
        ~Impl();

        boost::shared_ptr<SocketError> Listen(const Utils::HostInf &host);

        void Close();

        Utils::HostInf LocalHostInf() const
        {
            return m_hostInf;
        }

        void Delete();
    private slots:
        void ListenSlot(boost::shared_ptr<Utils::ThreadSyncEvent> lock, const Utils::HostInf &host);
        void DeleteSlot(boost::shared_ptr<Utils::ThreadSyncEvent>);            
    signals:
        void ListenSignal(boost::shared_ptr<Utils::ThreadSyncEvent> lock, const Utils::HostInf host);
        void DeleteSignal(boost::shared_ptr<Utils::ThreadSyncEvent>);  
    private:
        void incomingConnection (int socketDescriptor);
    private:
        MemberSocketThread m_memberThread;
        Utils::SafeRef<ITcpServerToUser> m_user;
        Utils::HostInf m_hostInf;
		boost::shared_ptr<SocketError> m_error;
    };
}

#endif

