#pragma once

#include "stdafx.h"
#include "Utils/HostInf.h"

namespace Utils
{
	QHostAddress GetDefLocalHost(QAbstractSocket::NetworkLayerProtocol ipVersion = 
		QAbstractSocket::IPv4Protocol);
		//QAbstractSocket::UnknownNetworkLayerProtocol);
	
	QString GetDefLocalIp(QAbstractSocket::NetworkLayerProtocol ipVersion = 
		QAbstractSocket::IPv4Protocol);

	HostInf GetDefLocalHostInf(int port);

	bool IsLocalIp(const QString &ip);

	bool IsSameLocalSubNet(const HostInf &addr1, const HostInf &addr2);

	bool IsSameLocalSubNet(const QString &ip1, const QString &ip2);
	
	bool IsSameLocalSubNet(const QHostAddress &ip1, const QHostAddress &ip2);

} // namespace Utils


