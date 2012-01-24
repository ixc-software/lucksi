#ifndef _HOST_INF_ADAPTER_H_
#define _HOST_INF_ADAPTER_H_

#include "stdafx.h"

namespace iNet
{
    class HostInfAdapter
    {
    public:
        static QHostAddress Convert(const std::string &str)
        {
            return QHostAddress(QString(str.c_str()));    
        }

        static std::string Convert(const QHostAddress &host)
        {
            return host.toString().toStdString();
        }
        
        static bool isIPv4Protocol(const std::string &ip)
        {
            return Convert(ip).protocol() == QAbstractSocket::IPv4Protocol;
        }

        static bool isIPv6Protocol(const std::string &ip)
        {
            return Convert(ip).protocol() == QAbstractSocket::IPv6Protocol;
        }
    };

};

#endif


