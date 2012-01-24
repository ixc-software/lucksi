#ifndef RTPTOTDMBUFFERSTAT_H
#define RTPTOTDMBUFFERSTAT_H

#include "Utils/StatisticElement.h"
#include "Utils/StatisticBase.h"

namespace TdmMng
{
    using Utils::StatElementForInt;

    struct RtpToTdmBufferStat : public Utils::StatisticBase<RtpToTdmBufferStat>
    {
        // статистика по входящим данным
        StatElementForInt BufferingRtpPackSize;             // статистика размеров пакетов записанных в буффер из Rtp
        int OverflowFifoCount;                              // overflow buffer fifo queue

        // статистика по исходящим данным        
        int SuccessReadFromBuffCount;                        // количество успешных операций чтения из буфера
        int TotalOutByte;                                  // общее количество прочтенных из буфера данных
        int SlipCount;                                      // количество проскальзываний
                
        // статистика по различным событиям
        int IncDepthCount;                                  // число шагов увеличения глубины буферизации
        int GoesBufferingCount;                             // количество переходов в режим буферизации

        // мета данные класса
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
