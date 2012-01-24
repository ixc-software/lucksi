#ifndef RTPTOTDMBUFFERSTAT_H
#define RTPTOTDMBUFFERSTAT_H

#include "Utils/StatisticElement.h"
#include "Utils/StatisticBase.h"

namespace TdmMng
{
    using Utils::StatElementForInt;

    struct RtpToTdmBufferStat : public Utils::StatisticBase<RtpToTdmBufferStat>
    {
        // ���������� �� �������� ������
        StatElementForInt BufferingRtpPackSize;             // ���������� �������� ������� ���������� � ������ �� Rtp
        int OverflowFifoCount;                              // overflow buffer fifo queue

        // ���������� �� ��������� ������        
        int SuccessReadFromBuffCount;                        // ���������� �������� �������� ������ �� ������
        int TotalOutByte;                                  // ����� ���������� ���������� �� ������ ������
        int SlipCount;                                      // ���������� ���������������
                
        // ���������� �� ��������� ��������
        int IncDepthCount;                                  // ����� ����� ���������� ������� �����������
        int GoesBufferingCount;                             // ���������� ��������� � ����� �����������

        // ���� ������ ������
        STATISTIC_META_INFO(RtpToTdmBufferStat)
        {
            meta.Add(inst.BufferingRtpPackSize,         "RtpToTdmBufferStat", StatElementForInt::LevAverage);
            meta.Add(inst.OverflowFifoCount,            "OverflowFifoCount");
            meta.Add(inst.SuccessReadFromBuffCount,     "SuccessReadFromBuffCount");
            meta.Add(inst.TotalOutByte,                 "TotalOutByte");
            meta.Add(inst.SlipCount,                    "SlipCount");
            meta.Add(inst.IncDepthCount,                "IncDepthCount");
            meta.Add(inst.GoesBufferingCount,           "GoesBufferingCount");
        }

        RtpToTdmBufferStat()
        {
            Clear();
        }

		template<class Data, class TStream>
		static void Serialize(Data &data, TStream &s)
        {
			StatElementForInt::Serialize(data.BufferingRtpPackSize, s);
            s
                << data.OverflowFifoCount
                << data.SuccessReadFromBuffCount
                << data.TotalOutByte
                << data.SlipCount
                << data.IncDepthCount
                << data.GoesBufferingCount;
        }


    }; 
} // namespace TdmMng

#endif
