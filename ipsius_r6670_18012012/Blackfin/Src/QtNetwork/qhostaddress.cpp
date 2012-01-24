
#include "stdafx.h"

#include "qhostaddress.h"
#include "Utils/StringParser.h"
#include "Utils/ManagedList.h"
#include "Utils/ErrorsSubsystem.h"
#include "QtCore/qchar.h"


namespace
{
    void RemoveAllSpaces(std::string &data)
    {
        int i = data.size() - 1;
        while ( i > 0  )
        {
            if ( QChar(data.at(i)).isSpace() ) data.erase(i, 1); 
            
            --i;
        }
    }

    // -------------------------------------------------------------

    template<class TNum>
    TNum StringToNum(const std::string &data, bool *ok)
    {
        std::istringstream ss(data);
        TNum res = 0;
        bool fail = (ss >> res).fail();
        if (ok != 0) *ok = !fail; 
       
        return (fail)? 0: res;
    }

    // -------------------------------------------------------------

    bool ParseIp4(const std::string &address, quint32 &outIp)
    {
        /*
        std::stringList ipv4 = address.split(QLatin1String("."));
        if (ipv4.count() != 4)
            return false;
    
        quint32 ipv4Address = 0;
        for (int i = 0; i < 4; ++i) {
            bool ok = false;
            uint byteValue = ipv4.at(i).toUInt(&ok);
            if (!ok || byteValue > 255)
                return false;
    
            ipv4Address <<= 8;
            ipv4Address += byteValue;
        }
    
        *addr = ipv4Address;
        return true;
        */

        // if ( !address.contains( QChar('.') ) ) return false;

        outIp = 0;
        std::string inIp(address);
        
        RemoveAllSpaces(inIp);
        
        Utils::ManagedList<std::string> parsedIp;
        Utils::StringParser(inIp, ".", parsedIp, false);

        if (parsedIp.Size() != 4) return false;
        
        for (int i = 0; i < parsedIp.Size(); ++i) 
        {
            bool ok = false;
            uint byteValue = StringToNum<uint>(*parsedIp[i], &ok);
    
            if ( !ok || (byteValue > 255) ) return false;
    
            outIp <<= 8;
            outIp += byteValue;
        }

        return true;
    }

    // -------------------------------------------------------------

    std::string ToStdString(quint32 ip)
    {
        /*
        s.sprintf("%d.%d.%d.%d", (i>>24) & 0xff, (i>>16) & 0xff,
                (i >> 8) & 0xff, i & 0xff);
        */

        std::ostringstream str;
        str << ( (ip>>24) & 0xff ) << '.' << ( (ip>>16) & 0xff ) << '.' 
            << ( (ip >> 8) & 0xff ) << '.' << ( ip & 0xff );

        return str.str();
    }
    
} // namespace

// -------------------------------------------------------------

class QHostAddress::QHostAddressImpl
{
    quint32 m_ipv4;    // a -- IPv4 address
    QAbstractSocket::NetworkLayerProtocol m_protocol;

    std::string m_ipString;
    bool m_isCorrectIpString;
    
public:
    QHostAddressImpl() : m_ipv4(0), m_protocol(QAbstractSocket::UnknownNetworkLayerProtocol), 
                m_isCorrectIpString(true)
    {
    }

    void Clear()
    {
        m_ipv4 = 0;
        m_protocol = QAbstractSocket::UnknownNetworkLayerProtocol;
        m_isCorrectIpString = true;
    }

    void SetAddress(quint32 ipv4)
    {
        m_ipv4 = ipv4;
        /*
        m_protocol = (ipv4 == 0)? 
            QAbstractSocket::UnknownNetworkLayerProtocol : QAbstractSocket::IPv4Protocol;
        */
        m_protocol = QAbstractSocket::IPv4Protocol;
        m_isCorrectIpString = true;
    }

    void SetAddress(const std::string &address)
    {
        Clear();

        // parse
        quint32 ip = 0;
        if ( !ParseIp4(address, ip) )
        {
            m_isCorrectIpString = false;
            return;
        }

        // set
        SetAddress(ip);
    }

    // if ok-parsed
    bool IsCorrectIpString() const
    {
        return m_isCorrectIpString;
    }
    
    QAbstractSocket::NetworkLayerProtocol getProtocol() const
    {
        return m_protocol;
    }

    quint32 getIpv4() const
    {
        return m_ipv4;
    }
    
};
    
// -------------------------------------------------------------

QHostAddress::QHostAddress() : m_pImpl(new QHostAddressImpl)
{
}

// -------------------------------------------------------------

QHostAddress::QHostAddress(quint32 ip4Addr)
    : m_pImpl(new QHostAddressImpl)
{
    setAddress(ip4Addr);
}

// -------------------------------------------------------------

QHostAddress::QHostAddress(const std::string &address)
    : m_pImpl(new QHostAddressImpl)
{
    setAddress(address);
}

// -------------------------------------------------------------

QHostAddress::QHostAddress(const QHostAddress &address)
    : m_pImpl( new QHostAddressImpl(*address.m_pImpl) )
{
}

// -------------------------------------------------------------

QHostAddress::QHostAddress(SpecialAddress address)
    : m_pImpl(new QHostAddressImpl)
{
    switch (address) 
    {
    case Null:
    case LocalHostIPv6:
    case AnyIPv6:
        break;
    case Broadcast:
        setAddress( std::string("255.255.255.255") );
        break;
    case LocalHost:
        setAddress( std::string("127.0.0.1") );
        break;
    case Any:
        setAddress( std::string("0.0.0.0") );
        break;
    default:
        ESS_ASSERT(0 && "Unknown SpecialAddress");
    }
}

// -------------------------------------------------------------

QHostAddress::~QHostAddress()
{
    delete m_pImpl;
}

// -------------------------------------------------------------

QHostAddress &QHostAddress::operator=(const QHostAddress &address)
{
    *m_pImpl = *address.m_pImpl;
    return *this;
}

// -------------------------------------------------------------

QHostAddress &QHostAddress::operator=(const std::string &address)
{
    setAddress(address);
    return *this;
}

// -------------------------------------------------------------

void QHostAddress::clear()
{
    m_pImpl->Clear();
}

// -------------------------------------------------------------

void QHostAddress::setAddress(quint32 ip4Addr)
{
    m_pImpl->SetAddress(ip4Addr);
}

// -------------------------------------------------------------

bool QHostAddress::setAddress(const std::string &address)
{
    m_pImpl->SetAddress(address);
    
    return m_pImpl->IsCorrectIpString(); 
}

// -------------------------------------------------------------

quint32 QHostAddress::toIPv4Address() const
{
    return m_pImpl->getIpv4();
}

// -------------------------------------------------------------

QAbstractSocket::NetworkLayerProtocol QHostAddress::protocol() const
{
    return m_pImpl->getProtocol();
}

// -------------------------------------------------------------

// (?)
std::string QHostAddress::toString() const
{
    if (m_pImpl->getProtocol() == QAbstractSocket::IPv4Protocol) 
    {
        quint32 ip = toIPv4Address();
        
        return ToStdString(ip);
    }

    return std::string();
}

// -------------------------------------------------------------

bool QHostAddress::operator==(const QHostAddress &other) const
{
    if (m_pImpl->getProtocol() == QAbstractSocket::IPv4Protocol)
    {
        return ( (other.m_pImpl->getProtocol() == QAbstractSocket::IPv4Protocol) 
                  && ( m_pImpl->getIpv4() == other.m_pImpl->getIpv4() ) );
    }
    
    return m_pImpl->getProtocol() == other.m_pImpl->getProtocol();
}

// -------------------------------------------------------------

bool QHostAddress::operator ==(SpecialAddress other) const
{
    /*
    QHostAddress otherAddress(other);
    if (m_pImpl->getProtocol() == QAbstractSocket::IPv4Protocol)
    {
        return ( (otherAddress.m_pImpl->getProtocol() == QAbstractSocket::IPv4Protocol)
                  && ( m_pImpl->getIpv4() == otherAddress.m_pImpl->getIpv4() ) );
    }

    return ( static_cast<int>(other) == static_cast<int>(Null) );
    */

    return operator==( QHostAddress(other) );
}

// -------------------------------------------------------------

bool QHostAddress::isNull() const
{
    return (m_pImpl->getProtocol() == QAbstractSocket::UnknownNetworkLayerProtocol);
}

// -------------------------------------------------------------

/*
uint qHash(const QHostAddress &key)
{
    return qHash(key.toString());
}
*/

