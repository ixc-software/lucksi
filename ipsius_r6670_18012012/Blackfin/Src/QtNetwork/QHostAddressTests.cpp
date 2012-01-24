
#include "stdafx.h"
#include "QtCore/QtCoreTests.h"
#include "QtNetwork/qhostaddress.h"


// test helpers
namespace
{
    // rawAddr -- to parse in QHostAddress
    // checkAddr -- to compare parsing result, 
    //              if parsing must fail, checkAddr must be empty 
    void TestCreatingFromString(const std::string &rawAddr, const std::string &checkAddr, 
                                bool okParsed)
    {
        // constructor
        QHostAddress ha(rawAddr);
        // TUT_ASSERT( ha.isNull() == checkAddr.isEmpty() );
        TUT_ASSERT( ha.isNull() == !okParsed );
        TUT_ASSERT( ha.toString() == checkAddr );
        
        // copy assignment
        ha.clear();
        ha = QHostAddress(rawAddr);
        // TUT_ASSERT( ha.isNull() == checkAddr.isEmpty() );
        TUT_ASSERT( ha.isNull() == !okParsed );
        TUT_ASSERT( ha.toString() == checkAddr );
        
        // setAddress()
        // TUT_ASSERT( QHostAddress().setAddress( rawAddr ) == ( !checkAddr.isEmpty() ) );
        TUT_ASSERT( QHostAddress().setAddress( rawAddr ) == okParsed);
    }

    // ------------------------------------------------

    // rawAddr -- to store in QHostAddress
    // checkAddr -- to compare parsing result, 
    //              if parsing must fail, checkAddr must be 0
    void TestCreatingFromQuint32(quint32 rawAddr, quint32 checkAddr)
    {
        // QHostAddress couldn't be isNull if it's been created using this methods

        // constructor
        QHostAddress ha(rawAddr);
        // if (checkAddr == 0) TUT_ASSERT( ha.isNull() );
        TUT_ASSERT( ha.toIPv4Address() == checkAddr );
        
        // copy assignment
        ha.clear();
        TUT_ASSERT( ha.isNull() );
        
        ha = QHostAddress(rawAddr);
        // if (checkAddr == 0) TUT_ASSERT( ha.isNull() );
        TUT_ASSERT( ha.toIPv4Address() == checkAddr );
    }

    // ------------------------------------------------

    quint32 MakeIp(ushort b0, ushort b1, ushort b2, ushort b3)
    {
        quint32 res = 0;
        res <<= 8;
        res |= b0;
        res <<= 8;
        res |= b1;
        res <<= 8;
        res |= b2;
        res <<= 8;
        res |= b3; 

        return res;
    }
    
} // namespace 

// ------------------------------------------------

// test
namespace
{
    /*
    QHostAddress();
    explicit QHostAddress(quint32 ip4Addr);
    explicit QHostAddress(const std::string &address);
    QHostAddress(const QHostAddress &copy);
    QHostAddress(SpecialAddress address);
    
    QHostAddress &operator=(const QHostAddress &other);
    QHostAddress &operator=(const std::string &address);

    quint32 toIPv4Address() const;
    std::string toString() const;

    bool isNull() const;
    void clear();

    void setAddress(quint32 ip4Addr);
    bool setAddress(const std::string &address);

    */
    
    void TestCreation()
    {
        // null
        TUT_ASSERT( QHostAddress().isNull() );
        TUT_ASSERT( QHostAddress(QHostAddress::Null).isNull() );

        // str
        TestCreatingFromString( std::string("125.126.100.8"), std::string("125.126.100.8"), true );
        TestCreatingFromString( std::string(" \t125\t.126  .100 .8  "), std::string("125.126.100.8"), true );
        TestCreatingFromString( std::string(".126.100.8"), std::string(), false );
        TestCreatingFromString( std::string("125.126.100.8."), std::string(), false );
        TestCreatingFromString( std::string("158.800.100.8"), std::string(), false );
        TestCreatingFromString( std::string("125.126.abc.8"), std::string(), false );
        TestCreatingFromString( std::string("0.0.0.0"), std::string("0.0.0.0"), true ); 

        // quint32
        TestCreatingFromQuint32( 0, 0 );
        quint32 ip = MakeIp(0, 120, 100, 150);
        TestCreatingFromQuint32( ip, ip );
        
        // copy
        QHostAddress ha("125.126.100.8");
        QHostAddress copyHa(ha);
        TUT_ASSERT( ha == copyHa );
    }

    // ------------------------------------------------

    /*
    bool operator ==(const QHostAddress &address) const;
    bool operator ==(SpecialAddress address) const;
    bool operator !=(const QHostAddress &address) const;
    bool operator !=(SpecialAddress address) const;
    bool operator ==(QHostAddress::SpecialAddress address1, const QHostAddress &address2);
    */

    void TestOperators()
    {
        TUT_ASSERT( QHostAddress(QHostAddress::Broadcast) 
                    == QHostAddress( std::string("255.255.255.255") ) );
        
        TUT_ASSERT( QHostAddress(QHostAddress::LocalHost) 
                    == QHostAddress( std::string("127.0.0.1") ) );
        
        TUT_ASSERT( QHostAddress(QHostAddress::Any) 
                    == QHostAddress( std::string("0.0.0.0") ) );

        TUT_ASSERT( QHostAddress(QHostAddress::Any) != QHostAddress() );

        TUT_ASSERT( QHostAddress("123.0.0.0") == QHostAddress("123.0.0.0") );
        TUT_ASSERT( QHostAddress("123.0.0.0") != QHostAddress("123.0.123.0") );
    }

    // ------------------------------------------------

    /*
    QAbstractSocket::NetworkLayerProtocol protocol() const;
    */

    void TestProtocol()
    {
        // empty constr
        TUT_ASSERT(QHostAddress().protocol() == QAbstractSocket::UnknownNetworkLayerProtocol);

        // ip4 str
        QHostAddress ha("123.0.0.0");
        TUT_ASSERT(ha.protocol() == QAbstractSocket::IPv4Protocol);

        // clear
        ha.clear();
        TUT_ASSERT(ha.protocol() == QAbstractSocket::UnknownNetworkLayerProtocol);
        
        // ip4 num
        ha.setAddress( MakeIp(100,100,100,0) );
        TUT_ASSERT(ha.protocol() == QAbstractSocket::IPv4Protocol);
        
        // 0 num
        quint32 ip = 0;
        ha.setAddress(ip);
        // TUT_ASSERT(ha.protocol() == QAbstractSocket::UnknownNetworkLayerProtocol);
        TUT_ASSERT(ha.protocol() == QAbstractSocket::IPv4Protocol);
        
        // ip4 wrong str
        ha.setAddress( std::string(".dse.100.") );
        TUT_ASSERT(ha.protocol() == QAbstractSocket::UnknownNetworkLayerProtocol);
        
        // special addr
        TUT_ASSERT(QHostAddress(QHostAddress::Any).protocol() == QAbstractSocket::IPv4Protocol);
    }

    
} // namespace

// ------------------------------------------------

namespace QtCoreTests
{
    void QHostAddressTests()
    {
        TestCreation();
        TestOperators();
        TestProtocol();
    }
    
} // namespace QtCoreTests
