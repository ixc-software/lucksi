#ifndef SSRCSTATISTIC_H
#define SSRCSTATISTIC_H

#include "Platform/PlatformTypes.h"
#include "Utils/StatisticElement.h"
#include "Utils/StatisticBase.h"

namespace iRtp
{
    using Utils::StatElementForInt;

    // ���������� ������ SsrcSeqCounter
    struct SsrcStatistic : public Utils::StatisticBase<SsrcStatistic>
    {        
        Platform::int64 RecvPacketCount;           // ���������� ������� �������� ������� (������������� ���)    
        Platform::int64 DropCount;                 // ���������� ������� ���������� ���������                        
        //Platform::ddword LostCount;                 // ���������� ������� ���������� ����� �������� ������ �� SeqCounterSettings::MaxMisOrder
        StatElementForInt RecvPackSizeStat;   // ���������� ���� �������� ���� ���������� � ���������� ������������

        Platform::int64 ReSynchroCounter;   // ������� ���������������
        Platform::int64 SeqFailureCounter;  // ������� ����� ��������� ������������������

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

    // ����� ���������� �� Rtp
    struct RtpStatistic : public Utils::StatisticBase<RtpStatistic>
    {
        // receiver statistic
        SsrcStatistic SsrcStat;     // �������� ���������� �� ���� ���������� ��� �� ����������
        int SsrcRecvCount;          // ���������� ���������� ��������������� ���������� �������������
        int SsrcRegistredCount;     // ���������� ��������������� ���������� ������������� � �������� ���� ��������� �������������
        
        // sender statistic
        StatElementForInt SendPackSizeStat;     // ���������� ���� �������� ���� ������������ �������������
        int SocketErrorCounter;                 // ������� ������ ������

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
