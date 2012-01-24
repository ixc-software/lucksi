#ifndef RTPCORESOCKET_H
#define RTPCORESOCKET_H

#include "Utils/MemoryPool.h"
#include "Lw/UdpSocket.h"

#include "iRtp/RtpSocketInterfaces.h"
#include "iRtp/IRtpInfra.h"

namespace iRtp
{
    class RtpCoreSocket : public IRtpToSocket
    {       
        Lw::UdpSocket m_lwSocket;    
        ISocketToRtp& m_rtp;  
        IRtpInfra& m_infra;                                            
        int m_ownPort;  // debug

        bool m_dstIsSet;
        HostInf m_dst;  

        HostInf m_local;
        
        // Lw::HostAddr m_LwDstCached; 

        // pre-allocated buff for recv packet, not smart ptr 'couse ownership goes to ISocketToRtp
        BidirBuffer* m_pRecvBuff;  

        std::string m_sendErrDesc;

        // try to read packet to m_pRecvBuff, return true on recv packet
        // called only from SocketPoll()
        bool ReadToRecvBuff(Utils::HostInf &addr);
        
    // IrtpToSocket impl
    private:                           
        
        const HostInf& getLocalAddr() const
        {
            return m_local;
        }

        void SetDstAddress(const HostInf& dstAddr);

        void TxData(BidirBuffer* pData, const HostInf& dstAddr);
        void TxData(BidirBuffer* pData);

        void SocketPoll(bool dropPackets, Utils::StatElementForInt &sockStat);

        /*
        bool BindToPort(int port)
        {
            ESS_ASSERT("PC only");

            return false;  // dummy
        } */

       
    public:
    
        //RtpCoreSocket(ISocketToRtp& rtp, ISocketToBlackfinInfra& infra, Utils::IFixedMemBlockPool& blockPool, Utils::IFixedMemBlockPool& shellPool, int port = UnknownPort);
        RtpCoreSocket(ISocketToRtp& rtp, IRtpInfra& infra, const HostInf& localAddr );
                        
        ~RtpCoreSocket() 
        { 
            delete m_pRecvBuff; 

            m_infra.FreeSocketPort( m_local.Port() );
        }

        /*
        static int GetCurrSockReadTime();
        static int GetMaxSockReadTime(); */

        static void CheckSocketState();
    };        

} // namespace iRtp

#endif
