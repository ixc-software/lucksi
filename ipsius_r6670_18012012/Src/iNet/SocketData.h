#ifndef __SOCKET_DATA_H__
#define __SOCKET_DATA_H__

#include "stdafx.h"

#include "ISocketId.h"
#include "Utils/HostInf.h"

namespace iNet
{

    // класс для передачи пакета данных через очередь сообщений
    class SocketData : boost::noncopyable
    {
    public:
        SocketData(const Utils::HostInf &host, const QByteArray &data) :
          m_peerInf(host),
          m_data(data) {}

        SocketId ID() const
        {
            return m_socketId;
        }

        void setSocketId(SocketId id)
        {
            m_socketId = id;
        }

        QByteArray &getData()
        {
            return m_data;
        }

        const QByteArray &getData() const
        {
            return m_data;
        }

        void setHostInf(const Utils::HostInf &host) 
        {
            m_peerInf = host;
        }
        
        const Utils::HostInf &getHostinf() const 
        {
            return m_peerInf;
        }

    private:
        SocketId m_socketId;
        Utils::HostInf  m_peerInf;
        QByteArray m_data;
    };

};
#endif

