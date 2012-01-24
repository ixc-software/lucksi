#ifndef __SOCKET_ERROR_H__
#define __SOCKET_ERROR_H__

#include "stdafx.h"
#include "ISocketId.h"

namespace iNet
{

    // класс для передачи ошибки через очередь сообщений
    class SocketError : boost::noncopyable
    {
    public:
        SocketError(QAbstractSocket::SocketError error, const QString &str) :
            m_error(error),
            m_str(str){}

        SocketId ID() const
        {
            return m_socketId;
        }

        void setSocketId(SocketId id)
        {
            m_socketId = id;
        }

        const QString &getErrorString() const
        {
            return m_str;
        }

        QAbstractSocket::SocketError getErrorIndex() const
        {
            return m_error;
        }
    private:
        SocketId m_socketId;
        QAbstractSocket::SocketError m_error;
        QString m_str;
    };

};
#endif

