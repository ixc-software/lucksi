#include "stdafx.h"
#include "GetDefLocalHost.h"

namespace 
{
	boost::shared_ptr<QNetworkAddressEntry>  GetNetworkAddressEntry(
		const QList<QNetworkAddressEntry> &addressEntries,
		const QHostAddress &addr)
	{
		boost::shared_ptr<QNetworkAddressEntry> res;
		for(QList<QNetworkAddressEntry>::const_iterator i = addressEntries.begin();
			i != addressEntries.end(); ++i)
		{
			if(i->ip() != addr) continue;

			res.reset(new QNetworkAddressEntry (*i));
			break;
		}
		return res;
	}

	boost::shared_ptr<QNetworkAddressEntry>  
		GetNetworkAddressEntry(const QList<QNetworkInterface> &interfaces,
			const QHostAddress &addr)
	{
		boost::shared_ptr<QNetworkAddressEntry> res;
		for(QList<QNetworkInterface>::const_iterator i = interfaces.begin();
			i != interfaces.end(); ++i)
		{
			res = GetNetworkAddressEntry(i->addressEntries(), addr);
			if(res != 0) break;
		}
		return res;
	}
};

namespace Utils
{
    QHostAddress GetDefLocalHost(QAbstractSocket::NetworkLayerProtocol ipVersion)
    {
        QList<QHostAddress> list(QNetworkInterface::allAddresses());        
        for(QList<QHostAddress>::ConstIterator i = list.begin(); i != list.end(); ++i)
        {
            if(*i == QHostAddress::Null || 
                *i == QHostAddress::LocalHost ||
                *i == QHostAddress::LocalHostIPv6 ||
                *i == QHostAddress::Broadcast ||
                *i == QHostAddress::Any ||
                *i == QHostAddress::AnyIPv6)
                continue;
			
			if (ipVersion == QAbstractSocket::UnknownNetworkLayerProtocol ||
				ipVersion == i->protocol()) 
				return *i;            
        }        

        return QHostAddress::LocalHost;
    }

	QString GetDefLocalIp(QAbstractSocket::NetworkLayerProtocol ipVersion)
	{
		return GetDefLocalHost(ipVersion).toString();
	}

	HostInf GetDefLocalHostInf(int port)
	{

		return HostInf(GetDefLocalHost(QAbstractSocket::IPv4Protocol).
			toString().toStdString(), port);
	}

	bool IsLocalIp(const QString &ip)
	{
		QList<QNetworkInterface> interfaces(QNetworkInterface::allInterfaces());
		return GetNetworkAddressEntry(interfaces, QHostAddress(ip)) != 0;
	}

	bool IsSameLocalSubNet(const HostInf &addr1, const HostInf &addr2)
	{
		return IsSameLocalSubNet(addr1.Address().c_str(), addr2.Address().c_str());
	}

	bool IsSameLocalSubNet(const QString &ip1, const QString &ip2)
	{
		return 	IsSameLocalSubNet(QHostAddress(ip1), QHostAddress(ip2)); 
	}

	bool IsSameLocalSubNet(const QHostAddress &addr1, const QHostAddress &addr2)
	{
		QList<QNetworkInterface> interfaces(QNetworkInterface::allInterfaces());
		boost::shared_ptr<QNetworkAddressEntry>  localAddressEntry(
			GetNetworkAddressEntry(interfaces, addr1));
		if(localAddressEntry == 0) localAddressEntry = GetNetworkAddressEntry(interfaces, addr2);
	
		return (localAddressEntry == 0) ? false :
			addr2.isInSubnet(addr1, localAddressEntry->prefixLength());
	}

} // namespace Utils

