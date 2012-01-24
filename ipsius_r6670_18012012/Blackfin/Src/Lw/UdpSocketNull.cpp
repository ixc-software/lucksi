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
        ESS_UNIMPLEMENTED;
        Init();
    }
    
    UdpSocket::UdpSocket() : m_handle(SOCK_DGRAM)
    {
        ESS_UNIMPLEMENTED;        
        Init();        
    }
    
    bool UdpSocket::Bind(int port)
    {
        ESS_UNIMPLEMENTED;        
        
        return true;        
    }
    

    UdpSocket::~UdpSocket()
    {
        ESS_UNIMPLEMENTED;        
    }

    bool UdpSocket::SendTo(const HostInf &addr, const void *pData, int dataSize, 
        std::string *pErrDesc)
    {
        ESS_UNIMPLEMENTED;
        
        return true;
    }

    bool UdpSocket::SendTo(const HostInf &addr, const Packet &data)
    {
        ESS_UNIMPLEMENTED;
        
        return true;        
    }

    void UdpSocket::SetDestination(const HostInf &addr)
    {
        ESS_UNIMPLEMENTED;
    }

    bool UdpSocket::Send(const Packet &data)
    {
        ESS_UNIMPLEMENTED;
        
        return true;        
    }
    
    bool UdpSocket::Send(const void *pData, int dataSize)
    {
        ESS_UNIMPLEMENTED;
        
        return true;        
    }

    bool UdpSocket::Recv(Packet &pack, HostInf &addr, bool *pPacketCut)
    {
        ESS_UNIMPLEMENTED;
        
        return true;
    }
    
    int UdpSocket::Recv(void *pData, size_t size, HostInf &addr, bool *pPacketCut)
    {
        ESS_UNIMPLEMENTED;
        
        return true;        
    }

	
	
}  // namespace Lw


