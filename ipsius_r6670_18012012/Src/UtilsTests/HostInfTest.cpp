
#include "stdafx.h"
#include "HostInfTest.h"

#include "Utils/HostInf.h"
#include "Utils/IntToString.h"
#include "Platform/Platform.h"

#include "Utils/DebugAllocationCounter.h"

// tests helpers
namespace
{
    using namespace Utils;

    const std::string CAnyAddr = "0.0.0.0";
    const std::string CValidAddr = "123.12.15.18";
    const dword CValidAddrDword = 302976123;
    const dword CValidAddrDwordReverted = 2064387858;
    const std::string CValidAddr2 = "111.01.215.17";
    const std::string CInvalidAddr = "_invalid_address_";
    const std::string CInvalidAddr2 = "888.777.999.000";
    
    const int CValidPort = 1256;
    const int CValidPort2 = 8569;
    const int CZeroPort = 0;
    const int CInitPort = -1;
    const int CInvalidPort = -2;
    const int CInvalidPort2 = 0x10000;

    const bool CAllowZeroPort = true;
    const bool CWithException = true;
    const bool CIsValid = true;
    const bool CIsEmpty = true;
    const bool CNetworkByteOrder = true;
    const bool CExceptionOnUpdate = true;
    
    // ------------------------------------------------------------

    class Validator
    {
        bool m_isEmpty;
        
        std::string m_addrStr;
        bool m_checkAddrAsStr;
        
        dword m_addrDword;
        bool m_checkAddrAsDword;

        int m_port;
        bool m_allowZeroPort;
        bool m_checkPort;

        bool m_isValid;
        bool m_withException;

        void InitDefault()
        {
            m_isEmpty = false;
            
            m_checkAddrAsStr = false;
            m_checkAddrAsDword = false;

            m_port = CInitPort;
            m_allowZeroPort = false;
            m_checkPort = false;

            m_isValid = false;
            m_withException = false;
        }
        
    public:
        explicit Validator() // should be invalid
        {
            InitDefault();
            m_isEmpty = true;
            m_checkPort = true; // port == init port
        }

        explicit Validator(bool isValid, bool withException)
        {
            if (withException) TUT_ASSERT(!isValid);

            InitDefault();
            m_isValid = isValid;
            m_withException = withException;
            
            if (m_isValid) return;
            
            m_checkAddrAsDword = true;
            m_checkAddrAsStr = true;
        }

        explicit Validator(int port, bool allowZeroPort, bool isValid, bool isEmpty)
        {
            InitDefault();
            m_isEmpty = isEmpty;
            m_port = port;
            m_allowZeroPort = allowZeroPort;
            m_checkPort = true;
            
            m_isValid = isValid;
        }
        
        explicit Validator(const std::string &addr, bool isValid, bool withException)
        {
            InitDefault();
            m_addrStr = addr;
            m_checkAddrAsStr = true;
            m_isValid = isValid;
            m_withException = withException;
        }

        explicit Validator(dword addr, bool isValid) // can't be exception
        {
            InitDefault();
            m_addrDword = addr;
            m_checkAddrAsDword = true;
            m_isValid = isValid;
        }

        explicit Validator(const std::string &addr, int port, bool allowZeroPort,
                           bool isValid, bool withException)
        {
            if (withException) TUT_ASSERT(!isValid);
            
            InitDefault();
            m_addrStr = addr;
            m_checkAddrAsStr = true;
            
            m_port = port;
            m_checkPort = true;
            m_allowZeroPort = allowZeroPort;

            m_isValid = isValid;
            m_withException = withException;
        }

        /*
        explicit Validator(dword addr, int port, bool allowZeroPort)
        {
            InitDefault();
            m_addrDword = addr;
            m_checkAddrAsDword = true;
            
            m_port = port;
            m_checkPort = true;
            m_allowZeroPort = allowZeroPort;
        }*/

        explicit Validator(const std::string &addr, dword addrDword, int port, 
                           bool allowZeroPort, bool isValid, bool withException)
        {
            InitDefault();
            m_addrStr = addr;
            m_checkAddrAsStr = true;

            m_addrDword = addrDword;
            m_checkAddrAsDword = true;
            
            m_port = port;
            m_checkPort = true;
            m_allowZeroPort = allowZeroPort;

            if (withException) TUT_ASSERT(!isValid);
            m_isValid = isValid;
            m_withException = withException;
        }

        void Check(const HostInf &h)
        {
            TUT_ASSERT(h.Empty() == m_isEmpty);
            TUT_ASSERT(h.IsValid(m_allowZeroPort) == m_isValid);
            
            if (m_checkPort) // no exceptions
            {
                TUT_ASSERT(h.Port() == m_port);
            }

            if (m_checkAddrAsStr) // can be exception
            {
                if (m_withException)
                {
                    bool wasException = false;
                    try
                    {
                        std::string addr = h.Address();
                    }
                    catch(HostInf::BadAddress &e)
                    {
                        wasException = true;
                    }
                    TUT_ASSERT(wasException);
                }
                else TUT_ASSERT(h.Address() == m_addrStr);
            }

            if (m_checkAddrAsDword) // can be exception
            {
                if (m_withException)
                {
                    bool wasException = false;
                    try
                    {
                        dword addr = h.InetAddr();
                    }
                    catch(HostInf::BadAddress &e)
                    {
                        wasException = true;
                    }
                    TUT_ASSERT(wasException);
                }
                else TUT_ASSERT(h.InetAddr() == m_addrDword);
            }
        }

        void UpdateAndCheck(int port, HostInf &h)
        {
            h.Port(port);
            Check(h);
        }

        void UpdateAndCheck(const std::string &addr, bool exceptionOnUpdate, HostInf &h)
        {
            if (!exceptionOnUpdate)
            {
                h.Address(addr);
                Check(h);
                return;
            }

            bool wasException = false;
            try
            {
                h.Address(addr);
            }
            catch (HostInf::BadAddress &e)
            {
                wasException = true;
            }
            TUT_ASSERT(wasException);
        }

        void UpdateAndCheck(dword addr, bool networkByteOrder, HostInf &h)
        {
            h.InetAddr(addr, networkByteOrder);
            Check(h);
        }

        void UpdateAndCheck(const std::string &addr, int port, 
                            bool exceptionOnUpdate, HostInf &h)
        {
            if (!exceptionOnUpdate)
            {
                h.Set(addr, port);
                Check(h);
                return;
            }

            bool wasException = false;
            try
            {
                h.Set(addr, port);
            }
            catch (HostInf::BadAddress &e)
            {
                wasException = true;
            }
            TUT_ASSERT(wasException);
        }
    };
    
    // ------------------------------------------------------------

    std::string HostInfToString(const std::string &addr, int port)
    {
        std::string res = addr;
        res += ":";
        res += Utils::IntToString(port);

        return res;
    }
    
} // namespace

// ------------------------------------------------------------

// tests
namespace 
{
    using namespace Utils;
    
    // ------------------------------------------------------------
    /*  HostInf();
        HostInf(int port);
        HostInf(const std::string &address, int port);
        
        bool Empty() const;
        bool IsValid(bool CAllowZeroPort = false) const; */
    
    void TestCreation()
    {
        // empty
        {
            HostInf h;
            Validator().Check(h);
        }
        // localhost + zero port
        {
            Validator(CZeroPort, CAllowZeroPort, CIsValid, !CIsEmpty).Check(HostInf(CZeroPort));
            Validator(CZeroPort, !CAllowZeroPort, !CIsValid, !CIsEmpty).Check(HostInf(CZeroPort));
        }
        // localhost + valid port
        {
            Validator(CValidPort, !CAllowZeroPort, CIsValid, !CIsEmpty).Check(HostInf(CValidPort));
        }
        // localhost + invalid port: < 0, >= 0x10000
        {
            Validator(CInvalidPort, CAllowZeroPort, !CIsValid, 
                      !CIsEmpty).Check(HostInf(CInvalidPort));
        }
        {
            Validator(CInvalidPort2, CAllowZeroPort, !CIsValid, 
                      !CIsEmpty).Check(HostInf(CInvalidPort2));
        }
        // valid address and port
        {
            HostInf h(CValidAddr, CValidPort);
            Validator(CValidAddr, CValidPort, !CAllowZeroPort, CIsValid, 
                      !CWithException).Check(h);
        }
        // invalid address and valid port
        {
            bool wasException = false;
            try
            {
                HostInf h(CInvalidAddr, CValidPort);
            }
            catch(HostInf::BadAddress &e)
            {
                wasException = true;
            }
            TUT_ASSERT(wasException);
            // Validator(CInvalidAddr, CValidPort, CAllowZeroPort, !CIsValid, CWithException).Check(h);
        }
        // empty address (considered as "0.0.0.0")
        {
            HostInf h("", CValidPort);
            Validator(CAnyAddr, CValidPort, !CAllowZeroPort, CIsValid, !CWithException).Check(h);
        }
        /* can't create host with invalid address
        // invalid address and port
        {
            HostInf h(CInvalidAddr, CInvalidPort);
            // CheckIfInvalidHost(h, &CInvalidPort);
            Validator(CInvalidAddr, CInvalidPort, CAllowZeroPort, !CIsValid, CWithException).Check(h);
        }*/
    }

    // ------------------------------------------------------------
    /*  HostInf(const HostInf &other);
        void operator=(const HostInf &other); */
    
    void TestCopying()
    {
        // empty to empty
        {
            HostInf h;
            HostInf h2(h);
            Validator().Check(h);
            Validator().Check(h2);

            HostInf h3 = h;
            Validator().Check(h);
            Validator().Check(h3);
        }
        // valid to empty
        {
            HostInf h(CValidPort);
            HostInf h2(h);
            Validator(CValidPort, CAllowZeroPort, CIsValid, !CIsEmpty).Check(h2);
            Validator(CValidPort, CAllowZeroPort, CIsValid, !CIsEmpty).Check(h);
            
            HostInf h3 = h;
            Validator(CValidPort, CAllowZeroPort, CIsValid, !CIsEmpty).Check(h3);
            Validator(CValidPort, CAllowZeroPort, CIsValid, !CIsEmpty).Check(h);
        }
        // empty to valid
        {
            HostInf h;
            HostInf h2(CValidPort);
            h2 = h;
            Validator().Check(h2);
            Validator().Check(h);
        }
        // valid to valid
        {
            HostInf h(CValidAddr, CValidPort);
            // CheckIfValidHost(h, &CValidPort, true, &CValidAddr);
            Validator(CValidAddr, CValidPort, CAllowZeroPort, CIsValid, !CWithException).Check(h);
            
            HostInf h2(CValidAddr2, CValidPort2);
            Validator(CValidAddr2, CValidPort2, CAllowZeroPort, CIsValid, !CWithException).Check(h2);

            h = h2;
            Validator(CValidAddr2, CValidPort2, CAllowZeroPort, CIsValid, !CWithException).Check(h2);
            Validator(CValidAddr2, CValidPort2, CAllowZeroPort, CIsValid, !CWithException).Check(h);
        }
        /*
        // can't create host with invalid address
        // valid to invalid
        {
            HostInf h(CValidAddr, CValidPort);
            Validator(CValidAddr, CValidPort,  CAllowZeroPort, CIsValid, !CWithException).Check(h);

            HostInf h2(CInvalidAddr, CInvalidPort);
            Validator(CIsValid, !CWithException).Check(h2);

            h2 = h;
            Validator(CValidAddr, CValidPort, CAllowZeroPort, CIsValid, !CWithException).Check(h);
            Validator(CValidAddr, CValidPort, CAllowZeroPort, CIsValid, !CWithException).Check(h2);
        }
        // invalid to valid: have exception during copying
        {
            HostInf h(CInvalidAddr, CInvalidPort);
            Validator(CIsValid, !CWithException).Check(h);
            
            HostInf h2(CValidAddr, CValidPort2);
            Validator(CValidAddr, CValidPort2, CAllowZeroPort, CIsValid, !CWithException).Check(h2);

            bool wasException = false;
            try
            {
                h2 = h;
            }
            catch (HostInf::BadAddress &e) { wasException = true; }
            TUT_ASSERT(wasException);
        }*/
    }

    // ------------------------------------------------------------
    /* void Clear(); */
    
    void TestClear()
    {
        // clear empty
        {
            HostInf h;
            Validator().Check(h);

            h.Clear();
            Validator().Check(h);
        }
        // clear valid
        {
            HostInf h(CValidAddr2, CValidPort2);
            Validator(CIsValid, !CWithException).Check(h);

            h.Clear();
            Validator().Check(h);
        }
        /* can't create host with invalid address
        // clear invalid
        {
            HostInf h(CInvalidAddr, CInvalidPort2);
            Validator(!CIsValid, CWithException).Check(h);

            h.Clear();
            Validator().Check(h);
        }
        */
    }

    // ------------------------------------------------------------
    /*  void Set(const std::string &addr, int port);
        void Port(int port);
        void Address(const std::string &s); */
  
    void TestSetValue()
    {
        // set valid to empty
        {
            HostInf h;
            Validator(CValidPort, CAllowZeroPort, !CIsValid, 
                      CIsEmpty).UpdateAndCheck(CValidPort, h);
        }
        {
            HostInf h;
            Validator(CValidAddr, !CIsValid, 
                      !CWithException).UpdateAndCheck(CValidAddr, !CExceptionOnUpdate, h);
        }
        {
            HostInf h;
            Validator(CValidAddr, CValidPort, CAllowZeroPort, CIsValid, 
                      !CWithException).UpdateAndCheck(CValidAddr, CValidPort, !CExceptionOnUpdate, h);
        }
        
        // set valid to valid
        {
            HostInf h(CValidAddr, CValidPort2);
            Validator(CValidPort, CAllowZeroPort, CIsValid, 
                      !CIsEmpty).UpdateAndCheck(CValidPort, h);
        }
        {
            HostInf h(CValidAddr2, CValidPort);
            Validator(CValidAddr, CIsValid, 
                      !CWithException).UpdateAndCheck(CValidAddr, !CExceptionOnUpdate, h);
        }
        {
            HostInf h(CValidAddr2, CValidPort2);
            Validator(CValidAddr, CValidPort, CAllowZeroPort, CIsValid, 
                      !CWithException).UpdateAndCheck(CValidAddr, CValidPort, !CExceptionOnUpdate, h);
        }
        // try set invalid to empty
        {
            HostInf h;
            Validator(CInvalidPort, CAllowZeroPort, !CIsValid, 
                      CIsEmpty).UpdateAndCheck(CInvalidPort, h);
        }
        {
            HostInf h;;
            Validator().UpdateAndCheck(CInvalidAddr2, CExceptionOnUpdate, h);
        }
        {
            HostInf h;
            Validator().UpdateAndCheck(CInvalidAddr2, CInvalidPort, CExceptionOnUpdate, h);
        }
        // try set invalid to valid
        {
            HostInf h(CValidAddr, CValidPort);
            Validator(CInvalidPort, CAllowZeroPort, !CIsValid, 
                      !CIsEmpty).UpdateAndCheck(CInvalidPort, h);
        }
        {
            HostInf h(CValidAddr, CValidPort);
            Validator().UpdateAndCheck(CInvalidAddr2, CExceptionOnUpdate, h);
        }
        {
            HostInf h(CValidAddr, CValidPort);
            Validator().UpdateAndCheck(CInvalidAddr2, CInvalidPort, CExceptionOnUpdate, h);
        }
        /* can't create host with invalid address
        // set valid to invalid
        {
            HostInf h(CInvalidAddr, CInvalidPort);
            Validator(CValidPort, CAllowZeroPort, !CIsValid).UpdateAndCheck(CValidPort, h);
        }
        {
            HostInf h(CInvalidAddr, CInvalidPort);
            Validator(CValidAddr, !CIsValid, !CWithException).UpdateAndCheck(CValidAddr, h);
        }
        {
            HostInf h(CInvalidAddr, CInvalidPort);
            Validator(CValidAddr, CValidPort, CAllowZeroPort, 
                      CIsValid, !CWithException).UpdateAndCheck(CValidAddr, CValidPort, h);
        }
        // set invalid to invalid
        {
            HostInf h(CInvalidAddr, CInvalidPort2);
            Validator(CInvalidPort, CAllowZeroPort, !CIsValid).UpdateAndCheck(CInvalidPort, h);
        }
        {
            HostInf h(CInvalidAddr, CInvalidPort2);
            Validator(CInvalidAddr2, !CIsValid, 
                      !CWithException).UpdateAndCheck(CInvalidAddr2, h);
        }
        {
            HostInf h(CInvalidAddr, CInvalidPort2);
            Validator(CInvalidAddr2, CInvalidPort, CAllowZeroPort, !CIsValid,
                      CWithException).UpdateAndCheck(CInvalidAddr2, CInvalidPort, h);
        }
        */
    }

    // ------------------------------------------------------------
    /*  int Port() const; */
    
    void TestAccessPort()
    {    
        HostInf h;
        Validator(CInitPort, CAllowZeroPort, !CIsValid, CIsEmpty).Check(h);
        
        h.Port(CValidPort);
        Validator(CValidPort, CAllowZeroPort, !CIsValid, CIsEmpty).Check(h);

        h.Port(CInvalidPort);
        Validator(CInvalidPort, CAllowZeroPort, !CIsValid, CIsEmpty).Check(h);

        h.Clear();
        Validator(CInitPort, CAllowZeroPort, !CIsValid, CIsEmpty).Check(h);
    }

    // ------------------------------------------------------------
    /*  const std::string& Address() const; // can throw BadAddress
        dword InetAddr() const;   // can throw BadAddress */
    
    void TestAccessAddress()
    {
        bool networkByteOrder = true;
        
        // from empty host
        {
            HostInf h;
            Validator().Check(h);
        }
        // from localhost
        {
            HostInf h(CValidPort);
            Validator(CAnyAddr, CIsValid, !CWithException).Check(h);
        }
        // from localhost with invalid port
        {
            HostInf h(CInvalidPort);
            Validator(CAnyAddr, !CIsValid, !CWithException).Check(h);
        }
        // all ---v with valid and invalid ports
        // save as valid str, get as str
        {
            HostInf h(CValidAddr, CValidPort);
            Validator(CValidAddr, CIsValid, !CWithException).Check(h);
            
            h.Port(CInvalidPort);
            Validator(CValidAddr, !CIsValid, !CWithException).Check(h);
        }
        // save as valid str, get as dword
        {
            HostInf h(CValidAddr, CValidPort);
            Validator(CValidAddrDword, CIsValid).Check(h);

            h.Port(CInvalidPort);
            Validator(CValidAddrDword, !CIsValid).Check(h);
        }
        // dword address always valid
        // save as dword, get as str
        {
            HostInf h;
            h.InetAddr(CValidAddrDword, networkByteOrder);
            Validator(CValidAddr, !CIsValid, !CWithException).Check(h);
            
            h.Port(CValidPort);
            Validator(CValidAddr, CIsValid, !CWithException).Check(h);
        }
        {
            HostInf h;
            h.InetAddr(CValidAddrDwordReverted, !networkByteOrder);
            Validator(CValidAddr, !CIsValid, !CWithException).Check(h);
            
            h.Port(CValidPort);
            Validator(CValidAddr, CIsValid, !CWithException).Check(h);
        }
        // save as dword, get as dword
        {
            HostInf h;
            h.InetAddr(CValidAddrDword, networkByteOrder);
            Validator(CValidAddrDword, !CIsValid).Check(h);

            h.Port(CValidPort);
            Validator(CValidAddrDword, CIsValid).Check(h);
        }
        {
            HostInf h;
            h.InetAddr(CValidAddrDwordReverted, !networkByteOrder);
            Validator(CValidAddrDword, !CIsValid).Check(h);
            
            h.Port(CValidPort);
            Validator(CValidAddrDword, CIsValid).Check(h);
        }
        /* can't create host with invalid address
        // save as invalid str, try to get as str
        {
            HostInf h(CInvalidAddr, CValidPort);
            Validator(CInvalidAddr, !CIsValid, CWithException).Check(h);
            
            h.Port(CInvalidPort);
            Validator(CInvalidAddr, !CIsValid, CWithException).Check(h);
        }
        // save as invalid str, try to get as dword
        {
            HostInf h(CInvalidAddr, CValidPort);
            Validator(CValidAddrDword, !CIsValid).Check(h);
            
            h.Port(CInvalidPort);
            Validator(CValidAddrDword, !CIsValid).Check(h);
        }
        */
    }

    // ------------------------------------------------------------
    /* void InetAddr(dword addr, bool networkByteOrder); */
    
    void TestAddressConvertion()
    {
        // even if we store address as string, 
        // it will be converted to dword if we call InetAddr() and otherwise

        // from string to dword
        {
            HostInf h(CValidAddr, CValidPort);
            Validator(CValidAddrDword, CIsValid).Check(h);
        }
        // from dword to string 
        // with network byte order
        {
            HostInf h(CValidPort);
            h.InetAddr(CValidAddrDword, true);
            Validator(CValidAddr, CIsValid, !CWithException).Check(h);
        }
        // with !network byte order
        {
            HostInf h(CValidPort);
            h.InetAddr(CValidAddrDwordReverted, false);
            Validator(CValidAddr, CIsValid, !CWithException).Check(h);
        }
    }

    // ------------------------------------------------------------
    /*  void ToString(std::string &s) const;
        std::string ToString() const; 
        // format: "127.0.0.0:1256", "(empty)", "(invalid)" */
    
    void TestToString()
    {
        // empty
        TUT_ASSERT(HostInf().ToString() == "(empty)");

        /*
        // invalid
        TUT_ASSERT(HostInf(CInvalidAddr, CInvalidPort).ToString() == "(invalid)");
        */
        
        // created from string address
        TUT_ASSERT(HostInf(CValidAddr, CValidPort).ToString() 
                   == HostInfToString(CValidAddr, CValidPort));

        // created from dword address
        std::string str;
        Platform::InetAddrToString(CValidAddrDword, str);
        {
            HostInf h(CValidPort);
            h.InetAddr(CValidAddrDword, true);
            TUT_ASSERT(h.ToString() == HostInfToString(str, CValidPort));
        }
        {
            HostInf h(CValidPort);
            h.InetAddr(CValidAddrDwordReverted, false);
            TUT_ASSERT(h.ToString() == HostInfToString(str, CValidPort));
        }
    }

    // ------------------------------------------------------------
    /*  bool operator == (const HostInf &other) const;
        bool operator != (const HostInf &other) const; */
    
    void TestComparison()
    {
        // with empty
        {
            TUT_ASSERT(HostInf() == HostInf());
            TUT_ASSERT(HostInf() != HostInf(CInvalidPort));
            // TUT_ASSERT(HostInf() != HostInf(CInvalidAddr, CInvalidPort));
            TUT_ASSERT(HostInf() != HostInf(CValidAddr, CValidPort));
        }
        // between valid
        {
            TUT_ASSERT(HostInf(CValidAddr, CValidPort) == HostInf(CValidAddr, CValidPort));
            TUT_ASSERT(HostInf(CValidAddr, CValidPort) != HostInf(CValidAddr2, CValidPort));
            TUT_ASSERT(HostInf(CValidAddr, CValidPort) != HostInf(CValidAddr, CValidPort2));
            TUT_ASSERT(HostInf(CValidAddr, CValidPort) != HostInf(CValidAddr2, CValidPort2));
            TUT_ASSERT(HostInf(CValidPort) == HostInf(CValidPort));
            TUT_ASSERT(HostInf(CValidPort) != HostInf(CValidPort2));
        }
        /* can't create host with invalid address
        // between invalid
        {
            bool wasException = false;
            try
            {
                bool res = (HostInf(CInvalidAddr, CInvalidPort) 
                            == HostInf(CInvalidAddr, CInvalidPort));
            }
            catch (HostInf::BadAddress &e) { wasException = true; }
            TUT_ASSERT(wasException);
        }
        {   
            bool wasException = false;
            try
            {
                bool res = (HostInf(CInvalidAddr, CInvalidPort) 
                            != HostInf(CInvalidAddr2, CInvalidPort));
                
            }
            catch (HostInf::BadAddress &e) { wasException = true; }
            TUT_ASSERT(wasException);
        }
        {
            TUT_ASSERT(HostInf(CInvalidAddr, CInvalidPort) != HostInf(CInvalidAddr, CInvalidPort2));
        }
        {
            TUT_ASSERT(HostInf(CInvalidAddr, CInvalidPort) != HostInf(CInvalidAddr2, CInvalidPort2));
        }
        */
        {  
            TUT_ASSERT(HostInf(CInvalidPort) == HostInf(CInvalidPort));
            TUT_ASSERT(HostInf(CInvalidPort) != HostInf(CInvalidPort2));
        }
        // between valid and invalid
        {
            TUT_ASSERT(HostInf(CValidAddr, CValidPort) != HostInf(CValidAddr, CInvalidPort));
        }
        /* can't create host with invalid address
        {
            TUT_ASSERT(HostInf(CValidAddr, CValidPort) != HostInf(CInvalidAddr, CValidPort));
            TUT_ASSERT(HostInf(CValidAddr, CValidPort) != HostInf(CInvalidAddr, CInvalidPort));
        }
        */
    }

    // ------------------------------------------------------------
    
    void TestFromString()
    {
        // Exception
        {
            bool wasException = false;
            try
            {
                HostInf::FromString("");
            }
            catch (HostInf::FromStringError &e)
            {
                wasException = true;
            }
            TUT_ASSERT(wasException);
        }
        // OK
        {
            TUT_ASSERT(HostInf::FromString("127.00.02.145:1256", HostInf::HostAndPort) 
                       == HostInf("127.00.02.145", 1256));
        }
        {
            TUT_ASSERT(HostInf::FromString(":1200", HostInf::HostAndPort) 
                       == HostInf(1200)); // empty addr
        }
        {
            HostInf inf;
            inf.Address("120.01.02.3");
            TUT_ASSERT(HostInf::FromString("120.01.02.3", HostInf::HostOnly).Address() 
                       == inf.Address());
        }
        {
            TUT_ASSERT(HostInf::FromString("1256", HostInf::PortOnly) == HostInf(1256));
        }
        {
            TUT_ASSERT(HostInf::FromString("1256", HostInf::Any) == HostInf(1256));
        }
        
        // Failed
        HostInf inf;
        {
            std::string err;
            TUT_ASSERT(!HostInf::FromString("120.01.02.3:", inf, HostInf::Any, &err));
            TUT_ASSERT(!err.empty());
        }
        {
            TUT_ASSERT(!HostInf::FromString(":", inf));
        }
        {
            TUT_ASSERT(!HostInf::FromString("120.", inf));
        }
        {
            TUT_ASSERT(!HostInf::FromString("20.01.02.3:12:12", inf));
        }
        {
            TUT_ASSERT(!HostInf::FromString("20.01.02.3:12.12", inf));
        }
        {
            TUT_ASSERT(!HostInf::FromString("port", inf));
        }
        // Failed: format
        {
            TUT_ASSERT(!HostInf::FromString("0.0.0.0:1256", inf, HostInf::HostOnly));
            TUT_ASSERT(!HostInf::FromString("0.0.0.0:1256", inf, HostInf::PortOnly));
        }
        {
            TUT_ASSERT(!HostInf::FromString("120.01.02.3", inf, HostInf::PortOnly));
            TUT_ASSERT(!HostInf::FromString("120.01.02.3", inf, HostInf::HostAndPort));
        }
        {
            TUT_ASSERT(!HostInf::FromString("1256", inf, HostInf::HostOnly));
            TUT_ASSERT(!HostInf::FromString("1256", inf, HostInf::HostAndPort));
        }
    }

    void StringToIPTest()
    {
        Platform::dword ip;

        TUT_ASSERT( HostInf::StringToIP("1.2.3.4", ip) && ip == 0x01020304 );

        TUT_ASSERT( HostInf::StringToIP("1.2.3.", ip) == false );
        TUT_ASSERT( HostInf::StringToIP("1.2.3", ip) == false );
        TUT_ASSERT( HostInf::StringToIP("", ip) == false );
        TUT_ASSERT( HostInf::StringToIP("301.1.2.3", ip) == false );
    }

    void StringToMacTest()
    {
        Platform::byte mac[6];

        TUT_ASSERT( HostInf::StringToMac("10:15:20:3f:4a:ff", mac, sizeof(mac)) );
        TUT_ASSERT( mac[0] == 0x10 && mac[1] == 0x15 && mac[2] == 0x20 && mac[3] == 0x3f 
            && mac[4] == 0x4a && mac[5] == 0xff );

        TUT_ASSERT( HostInf::StringToMac("", mac, sizeof(mac)) == false );
        TUT_ASSERT( HostInf::StringToMac("123", mac, sizeof(mac)) == false );
        TUT_ASSERT( HostInf::StringToMac("10:15:20:3f:4a:ffx", mac, sizeof(mac)) == false );
        TUT_ASSERT( HostInf::StringToMac("10:15:20:3f:4a:ff", mac, 4) == false );
        TUT_ASSERT( HostInf::StringToMac("10:15:20:3f:4a:ff", mac, 7) == false );
    }

    
} // namespace 

// ---------------------------------------------------

namespace UtilsTests
{
    void HostInfTest(bool silentMode)
    {
        int before = 0;
        
        if (Utils::AllocCounter::ModeEnabled()) before = Utils::AllocCounter().Get();
        else if (!silentMode)
        {
            std::cout << "Warning: Utils::AllocCounter disabled. " << std::endl;
        }
        
        TestCreation();
        TestCopying();
        TestClear();
        TestSetValue();
        TestAccessPort();
        TestAccessAddress();
        TestAddressConvertion();
        TestToString();
        TestComparison();
        TestFromString();

        StringToIPTest();
        StringToMacTest();

        if (Utils::AllocCounter::ModeEnabled())
        {
            int after = Utils::AllocCounter().Get();
            if (!silentMode)
            {
                std::cout << "HostInf test: alloc counter = " << before 
                        << "(+" << (after - before) << ")" << std::endl;
            }
            
            TUT_ASSERT((after - before) == 0);
        }

        if (!silentMode) std::cout << "HostInfTest: OK" << std::endl;
    }

    
} // namespace UtilsTests
