#pragma once

#include "stdafx.h"
#include "Platform/Platform.h"
#include "Platform/PlatformUtils.h"
#include "Utils/IntToString.h"
#include "Utils/StringParser.h"
#include "Utils/ManagedList.h"

namespace Utils
{
    using Platform::dword;
    using Platform::byte;

    // IP address + port  
    // Address stored as dword or string
    // Conversion dword <-> string do only on request (for optimisation)
    // IPv4 only support
    class HostInf
    {
        int m_port;

        // IP as dword in network byte order
        mutable bool  m_inetAddrResolved;
        mutable dword m_inetAddr;

        // IP as string 
        mutable std::string *m_pAddress;
        mutable byte m_addrBuff[sizeof(std::string)];

        enum 
        {
            CAnyAddr = 0,
        };

        // m_pAddress -> m_inetAddr
        bool ResolveInetAddr() const          
        {
            if (m_inetAddrResolved) return true;  // already resolved

            if (m_pAddress == 0) return false;

            bool fail;
            m_inetAddr = Platform::InetAddr(m_pAddress->c_str(), &fail);
            if (fail) return false;

            m_inetAddrResolved = true;
            return true;
        }

        // m_inetAddr -> m_pAddress
        bool ResolveAddress() const
        {
            if (m_pAddress != 0) return true;  // already resolved

            if (!m_inetAddrResolved) return false;

            m_pAddress = new(m_addrBuff) std::string();
            Platform::InetAddrToString(m_inetAddr, *m_pAddress);

            return true;
        }

        bool Equal(const HostInf &other) const  // can throw for invalid address
        {
            if (Empty() && other.Empty()) return true;
            if (Empty() != other.Empty()) return false;

            // here both is not empty
            if (Port() != other.Port()) return false;

            // both has string representation
            if ((m_pAddress != 0) && (other.m_pAddress != 0))
            {
                return (Address() == other.Address());
            }

            // compare as dword
            // (can force string -> dword convert, make this for more quick dword vs dword compare)
            return (InetAddr() == other.InetAddr());
        }


        void ClearAddress()
        {
            using std::string;

            if (m_pAddress != 0)
            {
                m_pAddress->~string();
                m_pAddress = 0;
            }
        }

        void SetFrom(const HostInf &other)
        {
            if (&other == this) return;

            Init();

            m_port = other.m_port;

            // begin with m_pAddress, 'couse it reset m_inetAddrResolved/m_inetAddr
            if (other.m_pAddress != 0) 
            {
                Address( other.Address() );  
            }

            m_inetAddrResolved = other.m_inetAddrResolved;
            m_inetAddr = other.m_inetAddr;

        }

        static bool PortFromString(const std::string &s, int &res, std::string *pErr = 0)
        {
            bool portOk = StringToInt(s, res);
            if ((!portOk) && (pErr != 0)) *pErr = "Invalid port";

            return portOk;
        }

        void Init()
        {
            m_port = -1;
            m_inetAddrResolved = false;
            m_pAddress = 0;
            m_inetAddr = 0; 
        }

    public:

        ESS_TYPEDEF(BadAddress);
        ESS_TYPEDEF(FromStringError);

        HostInf()
        {
            Init();
        }

        HostInf(const std::string &address, int port) 
        {
            Init();

            Address(address);
            Port(port);
         }

        HostInf(int port) 
        {
            Init();

            InetAddr(CAnyAddr, false);
            Port(port);
        }

        HostInf(const HostInf &other)
        {
            SetFrom(other);
        }

        ~HostInf()
        {
            ClearAddress();
        }

        void Clear()
        {
            ClearAddress();

            Init();
        }

        void operator=(const HostInf &other)
        {
            ClearAddress();

            SetFrom(other);
        }

        void Set(const std::string &addr, int port)
        {
            Address(addr);
            Port(port);            
        }


        // address
        const std::string& Address() const // can throw BadAddress
        {
            if (m_pAddress == 0)
            {
                if (!ResolveAddress()) ESS_THROW(BadAddress);
            }
            
            ESS_ASSERT(m_pAddress != 0);
            return *m_pAddress;
        }

        void Address(const std::string &s)  // can throw BadAddress
        {
            if (s.size() == 0)
            {
                Address("0.0.0.0");
                return;
            }

            m_pAddress = new(m_addrBuff) std::string(s);

            m_inetAddrResolved = false;            
            if (!ResolveInetAddr()) ESS_THROW(BadAddress);  // force resolve, string must be correct
        }

        // inet address
        dword InetAddr() const   // can throw BadAddress
        {
            if (!m_inetAddrResolved) 
            {
                if (!ResolveInetAddr()) ESS_THROW(BadAddress);
            }

            ESS_ASSERT(m_inetAddrResolved);
            return m_inetAddr;            
        }

        void InetAddr(dword addr, bool networkByteOrder)
        {
            if (!networkByteOrder) addr = Platform::RevertBytesInDword(addr);
            m_inetAddr = addr;
            m_inetAddrResolved = true;

            ClearAddress();
        }

        // port
        int Port() const
        {
            return m_port;
        }

        void Port(int port)
        {
            m_port = port;
        }

        // other         
        bool operator == (const HostInf &other) const 
        {
            return Equal(other);
        }

        bool operator != (const HostInf &other) const
        {
            return !Equal(other);
        }

        bool Empty() const
        {
            return (!m_inetAddrResolved && m_pAddress == 0);
        }            

		static bool IsValidPort(int port, bool allowZeroPort = false)
		{
			if (port == 0 && !allowZeroPort)     return false;
			return port >= 0 && port < 0x10000; 
		}

		bool IsValidIp() const
		{
			if (Empty()) return false;

			// addr
			// if (m_inetAddrResolved) do nothing, m_inetAddr always can converted to string

			return (m_inetAddrResolved ) ? true : ResolveInetAddr();

		}

        bool IsValid(bool allowZeroPort = false) const
        {
			return (!IsValidIp()) ? false : IsValidPort(m_port, allowZeroPort);
        }


        void ToString(std::string &s) const
        {
            s = ToString();
        }

        std::string ToString() const
        {
			if(Empty()) return "(empty)";

			if(!IsValidIp()) return "(invalid)";

			if(m_port != -1 && !IsValidPort(m_port)) return "(invalid)";
				
			std::ostringstream os;
            os << Address();
			if (m_port != -1) os << ":" << m_port;
            return os.str();
        }

        enum InStringExpected
        {
            Any = 0, // but correct one
            PortOnly = 1,
            HostOnly = 2,
            HostAndPort = 3,
        };

        static bool FromString(const std::string &s, 
                               HostInf &res,
                               InStringExpected format = Any,
                               std::string *pErr = 0)
        {
            res.Clear();

            if (s.empty())
            {
                if (pErr != 0) *pErr = "Empty string";
                return false;
            }
            
            int port = -1;
            std::string addr;
            InStringExpected gotFormat = Any;

            size_t sepIndex = s.find(':', 0);
            if (sepIndex == std::string::npos) 
            {
                bool hasDots = (s.find('.') != std::string::npos);
                if (hasDots)
                {
                    gotFormat = HostOnly;
                    addr = s;
                }
                else  
                {
                    gotFormat = PortOnly;
                    if (!PortFromString(s, port, pErr)) return false;
                }
            }
            else
            {
                gotFormat = HostAndPort;
                if (!PortFromString(s.substr(sepIndex + 1), port, pErr)) return false;
                addr = s.substr(0, sepIndex);
            }

            try 
            { 
                res.Set(addr, port); 
            } 
            catch (BadAddress &e)
            {
                if (pErr != 0) *pErr = "Invalid address";
                return false;
            }

            if ((format == Any) || (format == gotFormat)) return true;

            if (pErr != 0) *pErr = "Invalid format";
            return false;
        }
        
        static HostInf FromString(const std::string &s, 
                                  InStringExpected format = Any) // can throw FromStringError
        {
            HostInf res;
            std::string err;

            if (!FromString(s, res, format, &err)) ESS_THROW_MSG(FromStringError, err);

            return res;
        }

        // like inet_addr(), but use low-endian byte order
        static bool StringToIP(const std::string &s, Platform::dword &res)
        {
            Utils::ManagedList<std::string> list;
            Utils::StringParser(s, ".", list, false);

            if (list.Size() != 4) return false;

            res = 0;

            for(size_t i = 0; i < list.Size(); ++i)
            {
                int val;
                if ( !Utils::StringToInt(*(list[i]), val) ) return false;
                if ((val < 0) || ( val > 0xff)) return false;

                dword shift = 8 * (3 - i);
                res |= (val << shift);
            }

            return true;
        }

        // string format "01-02-03-04-05-06", numbers in hex with any single char separator
        static bool StringToMac(const std::string &s, Platform::byte *pResult, int resultSize)
        {
            const int CMacBytes = 6;

            if (resultSize != CMacBytes) return false;
            if (s.size() != (CMacBytes * 2) + CMacBytes - 1) return false;

            const char *p = s.c_str();

            for(int i = 0; i < CMacBytes; ++i)
            {
                Platform::dword val;            
                if (!Utils::HexStringToInt( &p[i * 3], val, false, 2)) return false;

                *pResult++ = val;
            }

            return true;
        }

    };


} // namespace Utils


