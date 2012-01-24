#ifndef __PROFILESETUP__
#define __PROFILESETUP__

#include "DataStream.h"

namespace SockTest
{

    // DataStream params + creation
    class ProfileBasic
    {
    protected:

        // test params
        int m_testTimeout;
        bool m_startTest;

        // параметры потока данных
        int m_sendSeed;
        int m_sendCount;
        int m_recvSeed;
        int m_recvCount;

    public:

        DataStream* CreateDataStream() const
        {
            return new DataStream(m_recvSeed, m_recvCount, m_sendSeed, m_sendCount, m_testTimeout, m_startTest); 
        }

    };

    // ---------------------------------------------------------

    // common params for all profiles
    class CommonParams
    {
    public:

        static const char* CPcHost() { return "192.168.0.1"; }
        static const char* CBfHost() { return "192.168.0.49"; }
    };

    // ---------------------------------------------------------

    // just consts for UDP client and server profile
    class UdpProfileBasic : public ProfileBasic
    {
    protected:

        static const int            CSrvPort = 36200;
        static const char*          CSrvHost() { return CommonParams::CPcHost(); } 

        static const int            CCliPort = 36200;

        static const int            CSrvSendSeed  = 1;
        static const int            CSrvSendCount = 1024;

        static const int            CCliSendSeed  = 2;
        static const int            CCliSendCount = 1024;

        static const int            CTestTimeout = 10 * 1000;
    };

    // ---------------------------------------------------------

    // Settings for UdpTestSrvside (PC)
    class UdpTestSrvsideProfile : public UdpProfileBasic
    {
        // сетевые параметры
        int m_ownPort;

        UdpTestSrvsideProfile() {}

    public:

        int OwnPort() const { return m_ownPort; }

        static UdpTestSrvsideProfile Create()
        {
            UdpTestSrvsideProfile profile;

            profile.m_ownPort = CSrvPort;

            // test params
            profile.m_testTimeout = CTestTimeout;
            profile.m_startTest = false;

            // data params
            profile.m_recvSeed = CCliSendSeed;
            profile.m_recvCount = CCliSendCount;

            profile.m_sendSeed = CSrvSendSeed;
            profile.m_sendCount = CSrvSendCount;
            
            return profile;
        }
    };

    // ---------------------------------------------------------

    // Settings for UdpCliTest (Blackfin)
    class UdpTestClientsideProfile : public UdpProfileBasic
    {
        // сетевые параметры
        int m_ownPort;
        int m_remotePort;
        std::string m_remoteHost;

        UdpTestClientsideProfile() {}

    public:

        int OwnPort() const             { return m_ownPort; }
        int RemotePort() const          { return m_remotePort; }
        std::string RemoteHost() const  { return m_remoteHost; }


        static UdpTestClientsideProfile Create()
        {
            UdpTestClientsideProfile profile;
            
            profile.m_ownPort = CCliPort;
            profile.m_remotePort = CSrvPort;
            profile.m_remoteHost = CSrvHost();

            // test params
            profile.m_testTimeout = CTestTimeout;
            profile.m_startTest = true;

            // data params
            profile.m_recvSeed = CSrvSendSeed;
            profile.m_recvCount = CSrvSendCount;

            profile.m_sendSeed = CCliSendSeed;
            profile.m_sendCount = CCliSendCount; 

            return profile;
        }

    };

    // ---------------------------------------------------------

    // consts for TcpSrvTestSrvsideProfile and 
    class TcpProfileBasic : public ProfileBasic
    {
    protected:

        static const int    CListenPort = 36300;
        static const char*  CListenHost()   { return CommonParams::CPcHost(); } 
        static const char*  CListenHostBf() { return CommonParams::CBfHost(); } 

        static const int            CSrvSendSeed  = 1;
        static const int            CSrvSendCount = 1024;

        static const int            CCliSendSeed  = 2;
        static const int            CCliSendCount = 1024;

        static const int            CTestTimeout = 10 * 1000;        

    };

    // ---------------------------------------------------------

    // settings for TcpSrvTestSrvside (PC)
    class TcpSrvTestSrvsideProfile : public TcpProfileBasic
    {
        int m_listenPort;

        TcpSrvTestSrvsideProfile() {}

    public:

        int ListenPort() const { return m_listenPort; }

        static TcpSrvTestSrvsideProfile Create()
        {
            TcpSrvTestSrvsideProfile profile;

            profile.m_listenPort = CListenPort;

            // test params
            profile.m_testTimeout = CTestTimeout;
            profile.m_startTest = false;

            // data params
            profile.m_recvSeed = CCliSendSeed;
            profile.m_recvCount = CCliSendCount;

            profile.m_sendSeed = CSrvSendSeed;
            profile.m_sendCount = CSrvSendCount; 

            return profile;
        }

    };

    // ---------------------------------------------------------

    // settings for TcpSockTestClientside (Blackfin)
    class TcpSockTestClientsideProfile : public TcpProfileBasic
    {
        int m_remotePort;
        std::string m_remoteHost;

        TcpSockTestClientsideProfile() {}

    public:

        int RemotePort() const          { return m_remotePort; }
        std::string RemoteHost() const  { return m_remoteHost; }

        static TcpSockTestClientsideProfile Create()
        {
            TcpSockTestClientsideProfile profile;

            profile.m_remotePort = CListenPort;
            profile.m_remoteHost = CListenHost();

            // test params
            profile.m_testTimeout = CTestTimeout;
            profile.m_startTest = true;

            // data params
            profile.m_recvSeed = CSrvSendSeed;
            profile.m_recvCount = CSrvSendCount;

            profile.m_sendSeed = CCliSendSeed;
            profile.m_sendCount = CCliSendCount; 

            return profile;
        }

    };

    // ---------------------------------------------------------

    // settings for TcpSockTestSrvside (PC)
    class TcpSockTestSrvsideProfile : public TcpProfileBasic
    {
        int m_remotePort;
        std::string m_remoteHost;

        TcpSockTestSrvsideProfile() {}

    public:

        int RemotePort() const          { return m_remotePort; }
        std::string RemoteHost() const  { return m_remoteHost; }

        static TcpSockTestSrvsideProfile Create()
        {
            TcpSockTestSrvsideProfile profile;

            profile.m_remotePort = CListenPort;
            profile.m_remoteHost = CListenHostBf();

            // test params
            profile.m_testTimeout = CTestTimeout;
            profile.m_startTest = false;

            // data params
            profile.m_recvSeed = CCliSendSeed;
            profile.m_recvCount = CCliSendCount;

            profile.m_sendSeed = CSrvSendSeed;
            profile.m_sendCount = CSrvSendCount; 

            return profile;
        }

    };

    // ---------------------------------------------------------

    // settings for TcpSrvTestClientside (Blackfin)
    class TcpSrvTestClientsideProfile : public TcpProfileBasic
    {
        int m_listenPort;

        TcpSrvTestClientsideProfile() {}

    public:

        int ListenPort() const { return m_listenPort; }

        static TcpSrvTestClientsideProfile Create()
        {
            TcpSrvTestClientsideProfile profile;

            profile.m_listenPort = CListenPort;

            // test params
            profile.m_testTimeout = CTestTimeout;
            profile.m_startTest = true;

            // data params
            profile.m_recvSeed = CSrvSendSeed;
            profile.m_recvCount = CSrvSendCount;

            profile.m_sendSeed = CCliSendSeed;
            profile.m_sendCount = CCliSendCount; 
            
            return profile;
        }

    };



}  // namespace SockTest

#endif

