#ifndef RTPPCSOCKET_H
#define RTPPCSOCKET_H

#include "RtpSocketInterfaces.h"
#include "iNet/MsgUdpSocket.h"

//#include "Utils/IFixedMemBlockPool.h"//?

namespace iRtp
{
    using Utils::BidirBuffer;

    class IRtpInfra;

    class RtpPcSocket : 
        public IRtpToSocket,
        public iNet::IUdpSocketToUser, 
        virtual public Utils::SafeRefServer
    {
        ISocketToRtp& m_rtp;        
        iNet::MsgUdpSocket m_socket;   

        HostInf m_dstAddr;
        bool m_dstAddrInited;   
        IRtpInfra& m_infra;

    // IRtpToSocket
    private:

        const HostInf& getLocalAddr() const;
        
        void TxData(BidirBuffer* pData, const HostInf& dstAddr);

        void SetDstAddress(const HostInf& dstAddr);
        void TxData(BidirBuffer* pData);

        // bool BindToPort(int port);

        void SocketPoll(bool dropPackets, Utils::StatElementForInt &sockStat)
        {
            ESS_ASSERT("Blackfin only!");
        }

    // iNet::IUdpSocketToUser impl
    private:

        void ReceiveData(boost::shared_ptr<iNet::SocketData> data);

        void SocketErrorOccur(boost::shared_ptr<iNet::SocketError> error);

    public:

        bool Bind(const HostInf &address);

        RtpPcSocket(iCore::MsgThread &thread, ISocketToRtp& rtp, IRtpInfra& infra);

    };
} // namespace iRtp

#endif
