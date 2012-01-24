#ifndef __RTPRECVRTPRECVBUFFERPROFILE__
#define __RTPRECVRTPRECVBUFFERPROFILE__

#include "SafeBiProto/ISerialazable.h"

namespace TdmMng
{    

    struct RtpRecvBufferProfile : 
        public SBProto::ISerialazable // public SBProto::SerialazableT<RtpRecvBufferProfile>
    {
        int MinBufferingDepth;      // ����������� (���������) ������� ������������ � �������
        int MaxBufferingDepth;      // ������������ ������� ������������ � �������
        int UpScalePercent;         // ���������� �������� ���� ���������� ������� ������������ � % ��� ��������� ��������� ������
        int QueueDepth;             // ������� FIFO ������ RTP �������
        int DropCount;              // ���������� ������� ������������� �� FIFO ��� ������������

        RtpRecvBufferProfile()
        {
            MinBufferingDepth   = -1;
            MaxBufferingDepth   = -1;
            UpScalePercent      = -1;
            QueueDepth          = -1;
            DropCount           = -1;
        }

		template<class Data, class TStream>
		static void Serialize(Data &data, TStream &s)
        {
            s 
                << data.MinBufferingDepth
                << data.MaxBufferingDepth
                << data.UpScalePercent
                << data.QueueDepth
                << data.DropCount;
        }

        bool IsCorrect() const
        {
            if (MinBufferingDepth < 1)      return false;
            if (MaxBufferingDepth < 1)      return false;
            if (UpScalePercent <= 100)      return false;
            if (QueueDepth < 1)             return false;
            if (DropCount < 1)              return false;

            if (MaxBufferingDepth < MinBufferingDepth) return false;
            if (MinBufferingDepth < DropCount) return false;

            return true;
        }

        std::string getAsString() const
        {
            std::ostringstream ss;

            ss << "MinBufferingDepth = " << MinBufferingDepth
                << ", MaxBufferingDepth = " << MaxBufferingDepth
                << ", UpScalePercent = " << UpScalePercent
                << ", QueueDepth = " << QueueDepth
                << ", DropCount = " << DropCount;

            return ss.str();
        }
    };
} // namespace TdmMng

#endif

