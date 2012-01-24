#include "stdafx.h"
#include "Utils/ErrorsSubsystem.h"
#include "UdpSocket.h"

namespace Lw
{
    enum { CMaxUdpPacketSize = 2 * 1024 };
    
    int UdpSocket::MaxUpdPackSize()
    {
    	return CMaxUdpPacketSize;
    }

    UdpSocket::UdpSocket(int port) : m_handle(SOCK_DGRAM)
    {        
        Init();

        if (!Bind(port))
        {
            // TODO -- change to throw - ?!
            ESS_HALT("Can't bind UDP socket");  
        }
    }
    
    UdpSocket::UdpSocket() : m_handle(SOCK_DGRAM)
    {
        Init();
    }
    
    bool UdpSocket::Bind(int port)
    {
        return m_handle.Bind(port);
    }
    

    UdpSocket::~UdpSocket()
    {
    }

    bool UdpSocket::SendTo(const HostInf &addr, const void *pData, int dataSize, 
        std::string *pErrDesc)
    {
        AssertBind();

        ESS_ASSERT(dataSize < CMaxUdpPacketSize);
        ESS_ASSERT( addr.IsValid() );

        sockaddr_in ra;
        HostInfToSockaddr(addr, ra);
    	    
    	int ret = lwip_sendto(m_handle.get(), const_cast<void*>(pData), dataSize, 0, (sockaddr*)&ra, sizeof(ra));    	

        if (ret < 0)
        {
            int errCode = GetSocketError();

            if (pErrDesc != 0)
            {
                std::ostringstream ss;
                ss << "UdpSock send fail, code " << errCode << "." << std::endl
                    << "Detination address:" << addr.ToString(); 

                *pErrDesc = ss.str();
            }

            return false;
        }

        if (ret != dataSize)
        {
            if (pErrDesc != 0)
            {
                std::ostringstream ss;
                ss << "UdpSock send size missmatch, send " << dataSize 
                    << ", reported " << ret;

                *pErrDesc = ss.str();
            }

            return false;
        }

    	m_packsSend++;

        return true;
    }

    bool UdpSocket::SendTo(const HostInf &addr, const Packet &data)
    {
        AssertBind();
        ESS_ASSERT(data.size() > 0);

        void *pData = PacketDataBegin(data);
        return SendTo(addr, pData, data.size());
    }

    void UdpSocket::SetDestination(const HostInf &addr)
    {
        m_dst = addr;
    }

    bool UdpSocket::Send(const Packet &data)
    {
        AssertBind();

        return SendTo(m_dst, data);
    }
    
    bool UdpSocket::Send(const void *pData, int dataSize)
    {
        AssertBind();

        return SendTo(m_dst, pData, dataSize);    	
    }

    bool UdpSocket::Recv(Packet &pack, HostInf &addr, bool *pPacketCut)
    {
        AssertBind();
        if (pack.size() < CMaxUdpPacketSize) pack.resize(CMaxUdpPacketSize);

        int count = Recv(PacketDataBegin(pack), pack.size(), addr, pPacketCut);
        if (count <= 0) return false;

        pack.resize(count);
        return true;
    }
    
    int UdpSocket::Recv(void *pData, size_t size, HostInf &addr, bool *pPacketCut)
    {
        AssertBind();
        ESS_ASSERT(size > 0);

        if (pPacketCut != 0) *pPacketCut = false;
        
        unsigned int flags = MSG_DONTWAIT;
        sockaddr_in ra;
        int raSize = sizeof(ra);
        
        int result = lwip_recvfrom(m_handle.get(), pData, size, flags,
        (sockaddr*)&ra, &raSize);
        
        if (result <= 0) return 0;

        if (result == size)
        {
            if (pPacketCut == 0) ESS_HALT("UDP cut!");
            *pPacketCut = true;
        }

        SockaddrToHostInf(ra, addr);
        /*
        addr.InetAddr(ra.sin_addr.s_addr, true);
        addr.Port( ntohs(ra.sin_port) ); */
                
        m_packsRecv++;

        return result;
    }

	
	
}  // namespace Lw


