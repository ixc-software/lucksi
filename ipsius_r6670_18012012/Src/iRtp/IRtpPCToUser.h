#ifndef RTP_PCUSERINTERFACES_H
#define RTP_PCUSERINTERFACES_H

#include "stdafx.h"
#include "Utils/BidirBuffer.h"
#include "Utils/IBasicInterface.h"
#include "RtpEvent.h"
#include "RtpHeaderForUser.h"
#include "RtpError.h"


namespace iRtp
{    
    using Platform::byte;

    typedef boost::shared_ptr<Utils::BidirBuffer> SharedBdirBuff;        

    // Высокоуровневый интерфейс пользователя Rtp. Направление: Rtp --> User.
    class IRtpPcToUser : public Utils::IBasicInterface
    {
    public:

        virtual void RxData(SharedBdirBuff buff, RtpHeaderForUser header) = 0;
        virtual void RxEvent(RtpEvent ev, dword timestamp) = 0;
        //virtual void RxNewPayload(RtpPayload payload) = 0;
        virtual void RtpErrorInd(RtpError er) = 0;
        virtual void NewSsrcRegistred(dword newSsrc) = 0;
    };
} // namespace iRtp

#endif
