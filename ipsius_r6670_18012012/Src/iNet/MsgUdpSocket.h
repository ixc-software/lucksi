#ifndef __MSG_UDP_SOCKET_H__
#define __MSG_UDP_SOCKET_H__

#include "stdafx.h"
#include "iCore/MsgThread.h"
#include "iCore/MsgObject.h"

#include "ISocketId.h"
#include "IUdpSocketToUser.h"
#include "UdpException.h"

#include "Utils/HostInf.h"
#include "SocketData.h"

namespace iNet 
{
    class SocketData;    
    class SocketError;
    class UdpSocket;
    
    using Utils::SafeRef;

    // Udp �����, ���������� ����� ������� ���������
    class MsgUdpSocket : 
        boost::noncopyable,
        public iCore::MsgObject, 
        public ISocketId,
        public IUdpSocketToUser 
    {
        typedef MsgUdpSocket T;        
    public:
        
        MsgUdpSocket(iCore::MsgThread &thread, SafeRef<IUdpSocketToUser> iUdpSocketToUser, 
            const Utils::HostInf &host); // !!! can throw UdpBindExcept
        
        MsgUdpSocket(iCore::MsgThread &thread, SafeRef<IUdpSocketToUser> iUdpSocketToUser);

        bool Bind(const Utils::HostInf &host);

    // ���������� IUdpSocket
    public:
        bool IsEqual(SocketId id) const;  

        // �������� ������ ������
        void SendData(boost::shared_ptr<SocketData> data);

        const Utils::HostInf &LocalInf() const;          

    // ��������� IUdpSocketToUser 
    private:

        // ��������� ��������� ������ ������
        void ReceiveData(boost::shared_ptr<SocketData> data);

        // ��������� ������
        void SocketErrorOccur(boost::shared_ptr<SocketError> error);

    private:    
        
        void MsgReceiveData(boost::shared_ptr<SocketData> data);
        void MsgSocketErrorOccur(boost::shared_ptr<SocketError> error);
        
        SocketId m_socketId;
        SocketId ID() const;

        SafeRef<IUdpSocketToUser> m_user; 
        boost::shared_ptr<UdpSocket> m_udpSocket; 
    };
};

#endif

