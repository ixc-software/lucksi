#ifndef RTPCORESOCKETINTERFACES_H
#define RTPCORESOCKETINTERFACES_H

/*
    ����������� �������������� ����������� ����� Rtp � Udp-�������.
*/

#include "Utils/BidirBuffer.h"
#include "Utils/IBasicInterface.h"
#include "Utils/StatisticElement.h"
#include "RtpError.h"

#include "Utils/HostInf.h"

namespace iRtp
{
    class IRtpToSocket;

    using Utils::HostInf;
    using Utils::BidirBuffer;
    typedef IRtpToSocket* SocketId; //class SocketId{};

    

    //----------------------------------------------------------------------------------------

    // ��������� Rtp �� ������� ������. ����������� Socket --> Rtp
    class ISocketToRtp : public Utils::IBasicInterface
    {
    public:        

        virtual void RxData(SocketId id, BidirBuffer* pData, const HostInf& dstAddr) = 0;                                
        virtual void ErrorInd(SocketId id, RtpError err) = 0;
    };

    //----------------------------------------------------------------------------------------

    // �������������� ��������� Udp ������ ��� Rtp. ����������� Rtp --> Socket.
    class IRtpToSocket : public Utils::IBasicInterface
    {
    public:
        
        bool IsEqual(SocketId id) const
        {
            return id == this;
        }

        virtual const HostInf& getLocalAddr() const  = 0;
        //virtual bool Bind(HostInf localAddr) = 0;

        // ���������� ���������� �������� �� �������� ������!!!
        virtual void TxData(BidirBuffer* pData, const HostInf& dstAddr) = 0;

        virtual void SetDstAddress(const HostInf& dstAddr) = 0;
        virtual void TxData(BidirBuffer* pData) = 0;

        // Blackfin only
        virtual void SocketPoll(bool dropPackets, Utils::StatElementForInt &sockStat) = 0;

        // PC only
        // virtual bool BindToPort(int port) = 0;
    };

} // namespace iRtp

#endif
