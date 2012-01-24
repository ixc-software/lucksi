#ifndef SSRCSTATISTIC_H
#define SSRCSTATISTIC_H

#include "Platform/PlatformTypes.h"
#include "Utils/StatisticElement.h"
#include "Utils/StatisticBase.h"

namespace iRtp
{
    using Utils::StatElementForInt;

    // статистика работы SsrcSeqCounter
    struct SsrcStatistic : public Utils::StatisticBase<SsrcStatistic>
    {        
        Platform::int64 RecvPacketCount;           // количество реально принятых пакетов (отбрасываемые втч)    
        Platform::int64 DropCount;                 // количество пакетов отклоненых счетчиком                        
        //Platform::ddword LostCount;                 // количество пакетов потерянных сетью Возможна ошибка на SeqCounterSettings::MaxMisOrder
        StatElementForInt RecvPackSizeStat;   // количество байт полезной инфы полученной и переданной пользователю

        Platform::int64 ReSynchroCounter;   // Счетчик ресинхронизаций
        Platform::int64 SeqFailureCounter;  // счетчик любых нарушений последовательности

        // meta info
        STATISTIC_META_INFO(SsrcStatistic)
        {
            meta.Add(inst.RecvPacketCount,          "RecvPacketCount");
            meta.Add(inst.DropCount,                "DropCount");
            meta.Add(inst.RecvPackSizeStat,         "RecvPackSizeStat", StatElementForInt::MaskSum);
            meta.Add(inst.ReSynchroCounter,         "ReSynchroCounter");
            meta.Add(inst.SeqFailureCounter,        "SeqFailureCounter");
        }

        SsrcStatistic()
        {
            Clear();
        }

        void operator+= (const SsrcStatistic &other)
        {
            SummWith(other);
        }

		template<class Data, class TStream>
			static void Serialize(Data &data, TStream &s)
        {
            s
                << data.RecvPacketCount
                << data.DropCount;
			
			StatElementForInt::Serialize(data.RecvPackSizeStat, s);
            
            s 
                << data.ReSynchroCounter
                << data.SeqFailureCounter;                
        }

        
    };   

    template<class TStream>
    TStream& operator << (TStream &s, const SsrcStatistic &stat)
    {
        s << stat.ToString();
        return s;
    }

    // ---------------------------------------------------------------------------------

    // общая статистика по Rtp
    struct RtpStatistic : public Utils::StatisticBase<RtpStatistic>
    {
        // receiver statistic
        SsrcStatistic SsrcStat;     // сумарная статистика по всем источникам или по последнему
        int SsrcRecvCount;          // количество полученных идентификаторов источников синхронизации
        int SsrcRegistredCount;     // количество идентификаторов источников синхронизации с которыми была выполнена синхронизация
        
        // sender statistic
        StatElementForInt SendPackSizeStat;     // количество байт полезной инфы отправленной пользователем
        int SocketErrorCounter;                 // счетчик ошибок сокета

        // meta info
        STATISTIC_META_INFO(RtpStatistic)
        {
            meta.Add(inst.SsrcStat,             "SsrcStat");
            meta.Add(inst.SsrcRecvCount,        "SsrcRecvCount");
            meta.Add(inst.SsrcRegistredCount,   "SsrcRegistredCount");
            meta.Add(inst.SendPackSizeStat,     "SendPackSizeStat", StatElementForInt::MaskSum);
            meta.Add(inst.SocketErrorCounter,   "SocketErrorCounter");
        }

        RtpStatistic()
        {
            Clear();
        }

		template<class Data, class TStream>
			static void Serialize(Data &data, TStream &s)
        {
			SsrcStatistic::Serialize(data.SsrcStat, s);
            s
                << data.SsrcRecvCount
                << data.SsrcRegistredCount;

			StatElementForInt::Serialize(data.SendPackSizeStat, s);

            s << data.SocketErrorCounter;
        }

    };
} // namespace iRtp

#endif
