#ifndef IRTPINFRA_H
#define IRTPINFRA_H

#include "stdafx.h"
#include "Utils/IBasicInterface.h"
#include "RtpSocketInterfaces.h"
#include "iLog/iLogSessionCreator.h"


namespace iRtp
{
    //---------------------------------
    
    ESS_TYPEDEF(NoFreePortPresent);// rename portbusy ?
    
    struct SocketPair
    {        
        boost::shared_ptr<IRtpToSocket> RtpSock;
        boost::shared_ptr<IRtpToSocket> RtcpSock;

        void Poll(bool dropPacket, Utils::StatElementForInt &sockStat)
        {
            if (RtpSock.get() != 0)  RtpSock->SocketPoll(dropPacket, sockStat);
            if (RtcpSock.get() != 0) RtcpSock->SocketPoll(dropPacket, sockStat);
        }
    };

    using Platform::dword;

    //---------------------------------    
    
    class IRtpInfra : public Utils::IBasicInterface
    {      
    
    public:

        virtual SocketPair CreateSocketPair(ISocketToRtp& socketOwner, bool useRtcp) = 0;

        virtual dword GenRndSSRC() = 0;    

        virtual BidirBuffer* CreateBuff(int offset) = 0;        
        virtual BidirBuffer* CreateBuff() = 0;  

        virtual void FreeSocketPort(int port) = 0;
    };
} // namespace iRtp

#endif
