
#ifndef QHOSTADDRESS_H
#define QHOSTADDRESS_H

#include "QtCore/qglobal.h"
#include "QtNetwork/qabstractsocket.h"

/*
isNull:
    QHostAddress();
    QHostAddress().clear();

!isNull:
    QHostAddress( std::string("0.0.0.0") );
    QHostAddress( QHostAddress::Any );
    QHostAddress().setAddress(0);
*/

class QHostAddress
{
    class QHostAddressImpl;
    
    QHostAddressImpl *m_pImpl;
    
public:
    enum SpecialAddress {
        Null,
        Broadcast,
        LocalHost,
        LocalHostIPv6, // ignored
        Any,
        AnyIPv6        // ignored
    };

    QHostAddress();
    explicit QHostAddress(quint32 ip4Addr);
    explicit QHostAddress(const std::string &address);
    QHostAddress(const QHostAddress &copy);
    QHostAddress(SpecialAddress address);
    ~QHostAddress();

    QHostAddress &operator=(const QHostAddress &other);
    QHostAddress &operator=(const std::string &address);

    void setAddress(quint32 ip4Addr);
    bool setAddress(const std::string &address);

    QAbstractSocket::NetworkLayerProtocol protocol() const;
    quint32 toIPv4Address() const;
    
    std::string toString() const;

    bool operator ==(const QHostAddress &address) const;
    bool operator ==(SpecialAddress address) const;
    inline bool operator !=(const QHostAddress &address) const { return !operator==(address); }
    inline bool operator !=(SpecialAddress address) const { return !operator==(address); }
    bool isNull() const;
    void clear();

};

// ------------------------------------------------------------

inline bool operator ==(QHostAddress::SpecialAddress address1, const QHostAddress &address2)
{ 
    return address2 == address1; 
}

// ------------------------------------------------------------

// uint qHash(const QHostAddress &key);



#endif // QHOSTADDRESS_H

