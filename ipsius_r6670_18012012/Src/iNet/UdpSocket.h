#ifndef __UDP_SOCKET_H__
#define __UDP_SOCKET_H__

#include "stdafx.h"
#include "Utils/SafeRef.h"
#include "Utils/HostInf.h"

namespace iNet
{
    class IUdpSocketToUser;
    class SocketData;
    class SocketError;
    using Utils::SafeRef;


    // Ѕазовый класс Udp сокетов
    class UdpSocket :  boost::noncopyable
    {
    public:
        
        UdpSocket(SafeRef<IUdpSocketToUser> iUdpSocketToUser, 
            const Utils::HostInf &host); // !!! can throw UdpBindExcept
        
       
        UdpSocket(SafeRef<IUdpSocketToUser> iUdpSocketToUser);

        ~UdpSocket();

        bool Bind(const Utils::HostInf &host);

        // передача пакета данных
        void SendData(boost::shared_ptr<iNet::SocketData> data);

        const Utils::HostInf &LocalInf() const;          
    // Implementation UdpSocket
    private:
        class Impl;
        Impl *m_impl;
    };
};
    
#endif

