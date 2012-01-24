#ifndef __NETUTILS__
#define __NETUTILS__

#include "stdafx.h"

#include "Platform/Platform.h"
#include "Utils/ErrorsSubsystem.h"
#include "Utils/HostInf.h"
#include "Utils/QtHelpers.h"
#include "BfEmul/E1PcEmul.h"
#include "iNet/SocketData.h"

namespace Lw
{
    using Platform::byte;
    using iNet::SocketData;

    typedef std::vector<byte> Packet;

    static void* PacketDataBegin(const Packet &v)
    {
        ESS_ASSERT(v.size() > 0);
        return (void*)&(v[0]);
    }

    static iCore::MsgThread& SocksThread()
    {
        return BfEmul::PcEmul::Instance().SocksThread();
    }

    // -----------------------------------------------------------
    
    /*
    class HostAddr
    {
        std::string m_address;

    public:

        HostAddr(QString s)
        {
            m_address = Utils::QStringToString(s);
        }

        HostAddr(const char *p = "0.0.0.0") : m_address(p)
        {
        }

        HostAddr(const std::string &s) : m_address(s)
        {
        }

        void set(const std::string &s)
        {
            m_address = s;
        }
        
        std::string get() const 
        {
            return m_address;
        }

        // called from RtpCoreSocket::ReadToRecvBuff()
        int getRaw() const
        {
            return 0; // ?
        }

    }; */



}  // namespace Lw

#endif

