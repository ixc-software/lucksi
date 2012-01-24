#ifndef RTP_BFUSERINTERFACES_H
#define RTP_BFUSERINTERFACES_H

/*
    Низкоуровневые интерфейсы между Rtp и пользователем.
*/

#include "Utils/BidirBuffer.h"
#include "Utils/IBasicInterface.h"
#include "Platform/PlatformTypes.h"
#include "RtpEvent.h"
#include "RtpError.h"
#include "RtpHeaderForUser.h"

namespace iRtp
{
    using Platform::dword;
    using Utils::BidirBuffer;    

    // Низкоуровневый интерфейс пользователя Rtp. Направление: Rtp --> User.
    class IRtpCoreToUser : public Utils::IBasicInterface
    {
    public:

        // реализация интерфейса отвечает за удаление буфера!!!
        virtual void RxData(BidirBuffer* pBuff, RtpHeaderForUser header) = 0;
        virtual void RxEvent(RtpEvent ev, dword timestamp) = 0;
        //virtual void RxNewPayload(RtpPayload payload) = 0;
        virtual void RtpErrorInd(RtpError er) = 0;

        // уведомление пользователя о установке синхронизации с новым источником
        virtual void NewSsrcRegistred(dword newSsrc) = 0;
    };

} // namespace iRtp

#endif
