#ifndef RTP_BFUSERINTERFACES_H
#define RTP_BFUSERINTERFACES_H

/*
    �������������� ���������� ����� Rtp � �������������.
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

    // �������������� ��������� ������������ Rtp. �����������: Rtp --> User.
    class IRtpCoreToUser : public Utils::IBasicInterface
    {
    public:

        // ���������� ���������� �������� �� �������� ������!!!
        virtual void RxData(BidirBuffer* pBuff, RtpHeaderForUser header) = 0;
        virtual void RxEvent(RtpEvent ev, dword timestamp) = 0;
        //virtual void RxNewPayload(RtpPayload payload) = 0;
        virtual void RtpErrorInd(RtpError er) = 0;

        // ����������� ������������ � ��������� ������������� � ����� ����������
        virtual void NewSsrcRegistred(dword newSsrc) = 0;
    };

} // namespace iRtp

#endif
